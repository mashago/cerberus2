# Cerberus2 — 多线程事件驱动服务框架

## 项目概述

C++11 实现的多线程事件驱动服务框架。核心是一个事件总线（`Cerberus`），管理多个服务，服务之间通过事件通信。

构建：CMake，输出静态库 `liblcerberus.a` + 可执行文件 `bin/cerberus` + 测试动态库 `lib/libservice_test.so`。

## 目录结构

```
cerberus2/
├── CMakeLists.txt              # 根 CMake（Debug，C++11，fPIC）
├── cerberus-src/
│   ├── CMakeLists.txt          # 核心库 + 可执行文件
│   ├── cerberus_main.cpp       # 入口：new Cerberus → start()
│   └── core/
│       ├── cerberus.h/cpp      # Cerberus 主控类（服务管理 + 事件路由）
│       ├── cerberus_service.h/cpp  # CerberusService 基类
│       ├── cerberus_thread.h/cpp   # 线程模型（share/monopoly/mgr）
│       ├── cerberus_event.h    # CerberusEvent（type, src_id, dest_id, session_id）
│       ├── cerberus_loader.h/cpp   # 动态库加载器（dlopen 封装）
│       └── cerberus_util.h/cpp # 工具（clear_container, 平台兼容 dl 函数）
├── service-src/
│   ├── CMakeLists.txt          # 编译为 libservice_test.so
│   ├── service_test.h          # TestService 等 4 个测试服务类
│   └── service_test.cpp        # 测试服务实现
├── bin/cerberus                # 编译产物
└── lib/libservice_test.so      # 编译产物
```

## 完整执行链路

### 启动流程

```
main()
  → Cerberus c;                              // 构造：new ShareThread + MonopolyThreadMgr + Loader
  → c.start()
    → service_loader->load("service_test")   // dlopen libservice_test.so
        → cerberus_create_service(c)         // 从 .so 中获取工厂函数
        → new TestService(c)
    → dispatch_share_thread_service(s)       // 注册 TestService(id=1) 到 share thread
        → _add_service: id=1, service_map, active()
            → event_list.push(EVENT_STARTUP), is_active=true
        → share_thread_mgr->add_service: thread_mgr=this, push active_service_list
    → set_thread_count(4)
    → share_thread_mgr->dispatch()           // 创建 4 个工作线程，然后 join（永不返回）
    → monopoly_thread_mgr->join()            // 等所有 monopoly 线程结束（永不返回）
```

### Share 线程事件循环

```
share_thread_run(thread_mgr, i)               // 4 个线程并发执行
  while(true):
    loop()
      → get_active_service()                  // 从 active_service_list 弹出一个
      → service->pop_event()                  // 从 service 的事件队列弹一个
      → service->handle_event(event)          // 处理事件
      → delete event
      → if service->is_release: delete service; return true
      → check_active(service)                 // 还有事件则放回 active_service_list
    if !loop():                                // 没拿到 service 或事件
      wait(active_service_cv)                  // 阻塞，等新服务/新事件
```

### 事件分发流程

```
service->handle_event 内部:
  → 创建新服务（dispatch_share_thread_service / dispatch_monopoly_thread_service）
  → push_event(new_event)
    → Cerberus::push_event: 锁 service_mtx, 查找 dest_service
    → dest_service->thread_mgr->push_event(service, event)
       → 锁 thread_mtx（share 或 monopoly）
       → service->push_event: 锁 service->mtx, event_list.push_back
       → 如果 !is_already_active: 加入 active_service_list
       → notify cv
```

## 线程模型

### 三种线程类型

| 类型 | 类 | 线程数 | 行为 |
|------|-----|--------|------|
| **Share** | `CerberusShareThread` | 4 个 | 竞争消费 `active_service_list` 中的服务。服务没有专属线程。 |
| **Monopoly Block** | `CerberusMonopolyThread` | 每个服务一个 | 专属线程，阻塞等待事件，逐事件处理（`loop()` 模式） |
| **Monopoly NonBlock** | `CerberusMonopolyThread` | 每个服务一个 | 专属线程，服务自己的 `dispatch()` 方法控制节奏 |

### 同步关键点

**Share 服务所有 event_list 写入都走 `share_thread_mtx`：**
- `CerberusShareThread::push_event` 持有 `share.thread_mtx` 后调 `service->push_event`（持有 `service->mtx`）
- `check_active` 持有 `share.thread_mtx` 读 `event_list`，因为所有写入者都被同一把 `thread_mtx` 序列化，安全

**Monopoly 服务推事件给 Share 服务也走 share 的锁：**
- `Cerberus::push_event` → `dest->thread_mgr->push_event` → 持有 `share.thread_mtx`
- Monopoly 之间互不影响，各有自己的 `thread_mtx`

**`Cerberus::push_event` 用 `service_mtx` 串行化所有事件路由查找**，是全局瓶颈。

## 已知问题

### 逻辑缺陷

1. **`CerberusLoader::load()` create_func 失败后 dl_map 残留无效句柄**（cerberus_loader.cpp:56-62）
   - 加载成功 → insert dl_map → 查找 create_func 失败 → dl_unload_lib → 但不从 dl_map 中移除
   - 下次再 load 同一服务名时命中 dl_map，拿到已卸载的句柄 → `dl_load_func` 使用悬空指针崩溃

2. **无优雅退出机制**
   - `start()` 中 `share_thread_mgr->dispatch()` 和 `monopoly_thread_mgr->join()` 都永不返回
   - `~Cerberus()` 永不被调用 → 所有析构清理无效
   - 程序只能通过 kill 退出（OS 回收资源，所以内存泄漏实际上不构成运行时问题）

### 内存管理（进程 kill 退出时无影响，但需优雅退出时要修）

3. `~Cerberus()` 只 delete `share_thread_mgr`，未 delete `monopoly_thread_mgr` 和 `service_loader`
4. `CerberusMonopolyThreadMgr::new_thread()` 分配的 `CerberusMonopolyThread` 对象无释放路径
5. `CerberusLoader` 无析构函数，dl_map 中的 dl 句柄永不 dlclose

### 测试代码问题

6. **`TestMonopolyNonBlockService::dispatch()` 忙等**（service_test.cpp:147-153）
   - 事件队列空时 `continue` 无 sleep → CPU 核 100%
7. **`all_service_vec`（service_test.cpp:20）读写竞争**
   - TestService 在 share 线程写入，monopoly 线程启动后立即从自己的线程读取 `random_service()`，存在读-写并发

### 设计局限

9. `Cerberus::push_event` 全局一把 `service_mtx` → 高并发下性能瓶颈
10. `CerberusService::release()` 中 TODO：清除事件列表但不通知发送方事件被丢弃
11. CMake 使用废弃的 `FILE(GLOB ...)`，新增源文件需手动重新 cmake

## 关键设计特点

- **服务通过 `c->push_event(event)` 互相发消息**，由 Cerberus 统一路由
- **`CerberusEvent` 是堆分配，接收方处理完毕后 delete**
- **服务可被 release**（`release_service` 从 `service_map` 移除，后续指向该服务的事件投递静默失败）
- **服务 creator 是 C 链接工厂函数** `cerberus_create_service(Cerberus*)`，由 `CerberusLoader` 通过 dlopen/dlsym 调用
- **Share thread 线程数可配**，通过 `set_thread_count()` 在 dispatch 前设置

# Cerberus2 — 多线程事件驱动服务框架

## 项目概述

C++11 实现的多线程事件驱动服务框架。核心是一个事件总线（`Cerberus`），管理多个服务，服务之间通过事件通信。

构建：CMake，输出静态库 `liblcerberus.a` + 可执行文件 `bin/cerberus` + 测试动态库 `lib/libservice_test.so`。

## 目录结构

```
cerberus2/
├── CMakeLists.txt
├── cerberus-src/
│   ├── CMakeLists.txt
│   ├── cerberus_main.cpp
│   └── core/
│       ├── cerberus.h/cpp          # Cerberus 主控类（服务管理 + 事件路由）
│       ├── cerberus_service.h/cpp  # CerberusService 基类
│       ├── cerberus_thread.h/cpp   # 线程模型（share/monopoly/mgr）
│       ├── cerberus_event.h        # CerberusEvent（type, src_id, dest_id, session_id）
│       ├── cerberus_loader.h/cpp   # 动态库加载器（dlopen 封装）
│       ├── cerberus_util.h/cpp     # 工具（clear_container, 平台兼容 dl 函数）
│       ├── cerberus_log.h/cpp      # 日志（ERROR/WARN/INFO/DEBUG 四级，输出到 stderr）
│       └── cerberus_rwlock.h       # pthread_rwlock_t RAII 封装
├── service-src/
│   ├── CMakeLists.txt
│   ├── service_test.h              # TestService 等 4 个测试服务类
│   └── service_test.cpp
├── bin/cerberus                    # 编译产物
└── lib/libservice_test.so          # 编译产物
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
      → check_active(service)                 // 还有事件则放回 active_service_list，否则设 is_active=false
    if !loop():                                // 没拿到 service 或事件
      wait(active_service_cv)                  // 阻塞，等新服务/新事件
```

### 事件分发流程

```
service->handle_event 内部:
  → 创建新服务（dispatch_share_thread_service / dispatch_monopoly_thread_service）
  → push_event(new_event)
    → Cerberus::push_event: 读锁 service_rwlock, 查找 dest_service
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
| **Share** | `CerberusShareThread` | 可配（默认 4） | 竞争消费 `active_service_list` 中的服务。服务没有专属线程。 |
| **Monopoly Block** | `CerberusMonopolyThread` | 每个服务一个 | 专属线程，阻塞等待事件，逐事件处理（`loop()` 模式） |
| **Monopoly NonBlock** | `CerberusMonopolyThread` | 每个服务一个 | 专属线程，服务自己的 `dispatch()` 方法控制节奏 |

### 同步关键点

**Share 服务所有 event_list 写入都走 `thread_mtx`：**
- `CerberusShareThread::push_event` 持有 `thread_mtx` 后调 `service->push_event`（持有 `service->mtx`）
- `check_active` 持有 `thread_mtx` 直接读 `event_list` 和写 `is_active`，所有写入者都被同一把 `thread_mtx` 序列化

**Monopoly 服务推事件给 Share 服务也走 share 的锁：**
- `Cerberus::push_event` → `dest->thread_mgr->push_event` → 持有 `thread_mtx`
- Monopoly 之间互不影响，各有自己的 `thread_mtx`

**`Cerberus::push_event` 用 `service_rwlock` 保护 `service_map`：**
- 多个 `push_event` 可并发读（`ReadLockGuard`）
- `_add_service` / `release_service` 用 `WriteLockGuard` 串行化

## 已知问题

### 逻辑缺陷

1. **无优雅退出机制**
   - `start()` 中 `share_thread_mgr->dispatch()` 和 `monopoly_thread_mgr->join()` 都永不返回
   - `~Cerberus()` 永不被调用 → 所有析构清理无效
   - 程序只能通过 kill 退出（OS 回收资源，所以内存泄漏不构成运行时问题）

2. **`TestMonopolyNonBlockService::dispatch()` 忙等**（service_test.cpp:139-158）
   - 事件队列空时 `continue` 无 sleep → CPU 核 100%
   - NonBlock 服务的 dispatch 不检查 `is_release`，即使 release 了也继续死循环

3. **`all_service_vec` 读写竞争**（service_test.cpp:22）
   - TestService 在 share 线程写入，monopoly 线程启动后通过 `random_service()` 读取，无锁保护

4. **`CerberusService::thread_mgr` 未在构造列表初始化**（cerberus_service.cpp:17-21）
   - 初始化列表没有 `thread_mgr(nullptr)`，当前调用链安全但属于潜在隐患

5. **`_get_dl_name` 静态缓冲区可能溢出**（cerberus_loader.cpp:21）
   - `snprintf(path, 512, "lib%s.so", service_name)` 对 service_name 长度无保护

6. **`CerberusService::release()` 不通知发送方事件被丢弃**（cerberus_service.cpp:86）
   - TODO 注释，被 release 的服务未处理事件被直接清理，发送方不知情

### 内存管理

7. `~Cerberus()` 只 delete `share_thread_mgr`，未 delete `monopoly_thread_mgr` 和 `service_loader`（cerberus.cpp:27-28）
8. `CerberusMonopolyThreadMgr::new_thread()` 分配的 `CerberusMonopolyThread` 对象无释放路径（cerberus_thread.cpp:210）
9. `CerberusLoader` 无析构函数，dl_map 中的 dl 句柄永不 dlclose（cerberus_loader.h:10-19）

### 设计局限

10. `CerberusService::release()` 中 TODO：不通知发送方事件被丢弃（cerberus_service.cpp:86）
11. 缺少 `override` 关键字，基类虚函数签名变更时子类不会报错（service_test.h:11,18,25,33-34）
12. `CerberusService::push_event` 返回值语义混乱：返回 `true` 表示"已活跃"，调用方用 `!is_already_active` 判断（cerberus_service.cpp:47-57）
13. 无信号处理，只能 `kill -9` 退出（cerberus_main.cpp:8-16）
14. 服务 ID 用 `int` 递增无上限检查，长期运行可能溢出（cerberus.cpp:32-33）
15. 无单元测试框架，只有 service_test.so 做端到端集成测试
16. CMake 使用 `FILE(GLOB ...)`，新增源文件需手动重新 cmake

## 已修复问题

| # | 问题 | 修复 |
|----|------|------|
| — | `CerberusLoader::load()` dl_map 残留无效句柄 | dl_unload_lib 后追加 dl_map.erase(path) |
| — | `Cerberus::push_event` 全局互斥锁瓶颈 | 改为 pthread_rwlock_t 读写锁 |
| — | `check_active` 双重持锁 | 去掉内层冗余 service->mtx |
| — | `CerberusService::pop_events` 逐元素拷贝 | 用 splice 替换 O(1) |
| — | printf 遍布热路径 | 新建 Log 类，ERROR/WARN/INFO/DEBUG 四级 |
| — | `cerberus_loader.h` 缺少 `#include <string>` | 已补充 |
| — | `CerberusShareThread::thread_count` 未初始化 | 构造列表初始化为 0 |
| — | `_WIN32` / `WIN32` 宏不一致 | 统一为 `_WIN32` |
| — | `cerberus.cpp` / `cerberus_service.cpp` 残留无用 C 头文件 | 已清理 |
| — | `clear_container` 冗余 `*iter = nullptr` | 已删除 |

## 关键设计特点

- **服务通过 `c->push_event(event)` 互相发消息**，由 Cerberus 统一路由
- **`CerberusEvent` 是堆分配，接收方处理完毕后 delete**
- **服务可被 release**（`release_service` 从 `service_map` 移除，后续指向该服务的事件投递静默失败）
- **服务 creator 是 C 链接工厂函数** `cerberus_create_service(Cerberus*)`，由 `CerberusLoader` 通过 dlopen/dlsym 调用
- **Share thread 线程数可配**，通过 `set_thread_count()` 在 dispatch 前设置
- **日志**：`Log::error/warn/info/debug()`，输出到 stderr，可通过 `Log::set_level()` 控制级别

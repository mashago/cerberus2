
#include "cerberus_thread.h"
#include "cerberus_event.h"
#include "cerberus_service.h"

CerberusThread::CerberusThread() : is_running(true)
{
}

CerberusThread::~CerberusThread()
{
}

///////////////////////////////////////////////////////

CerberusShareThread::CerberusShareThread()
{
}

void share_thread_run(CerberusShareThread* thread_mgr, int i)
{
	while (true)
	{
		if (!thread_mgr->loop())
		{
		    printf("thread sleep i=%d\n", i);
			std::unique_lock<std::mutex> lock(thread_mgr->thread_mtx);
			thread_mgr->active_service_cv.wait(lock, [thread_mgr](){ return !thread_mgr->empty_active_list(); });
		}
	}
}

void CerberusShareThread::dispatch()
{
    std::list<std::thread> tl;
	for (int i = 0; i < thread_count; ++i)
	{
		printf("i=%d\n", i);
		tl.push_back(std::thread(share_thread_run, this, i));
	}

	for (auto &t : tl)
	{
		t.join();
	}
}

bool CerberusShareThread::loop()
{
	CerberusService* service = get_active_service();
	if (!service)
	{
		return false;
	}

	CerberusEvent* event = service->pop_event();
	if (!event)
	{
		check_active(service);
		return false;
	}

	// handle event
	service->handle_event(event);
	delete event;
    if (service->is_release)
    {
        delete service;
        return true;
    }

	check_active(service);
	return true;
}

bool CerberusShareThread::push_event(CerberusService* service, CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	bool is_already_active = service->push_event(event);
	if (!is_already_active)
	{
		active_service_list.push_back(service);
	}
	active_service_cv.notify_one();
	return true;
}

void CerberusShareThread::set_thread_count(int thread_count)
{
    this->thread_count = thread_count;
}

void CerberusShareThread::add_service(CerberusService* service)
{
	service->thread_mgr = this;
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	active_service_list.push_back(service);
	active_service_cv.notify_all();
}

void CerberusShareThread::check_active(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	if (!service->event_list.empty())
	{
		active_service_list.push_back(service);
		active_service_cv.notify_one();
		return;
	}

	std::unique_lock<std::mutex> lock_small(service->mtx);
	if (service->event_list.empty())
	{
		service->is_active = false;
	}
	else
	{
		active_service_list.push_back(service);
		active_service_cv.notify_one();
	}
}

bool CerberusShareThread::empty_active_list()
{
    return active_service_list.empty();
}

CerberusService* CerberusShareThread::get_active_service()
{
	CerberusService* service = nullptr;
	std::unique_lock<std::mutex> lock(thread_mtx);
	if (!active_service_list.empty())
	{
		service = active_service_list.front();
		active_service_list.pop_front();
	}
	return service;
}

///////////////////////////////////////////////////

CerberusMonopolyThread::CerberusMonopolyThread(CerberusService* service) : service(service)
{
	service->thread_mgr = this;
}

void monoploy_thread_block_run(CerberusMonopolyThread* thread_mgr)
{
	while (thread_mgr->is_running)
	{
		if (!thread_mgr->loop())
		{
			std::unique_lock<std::mutex> lock(thread_mgr->thread_mtx);
			thread_mgr->active_service_cv.wait(lock, [thread_mgr](){ return !thread_mgr->service->event_list.empty(); });
		}
	}
}

void monoploy_thread_non_block_run(CerberusMonopolyThread* thread_mgr)
{
	// only run servier dispatch, just let service get its event
	thread_mgr->service->dispatch();
}

void CerberusMonopolyThread::dispatch()
{
	if (service->is_block)
	{
		td = std::thread(monoploy_thread_block_run, this);
	}
	else
	{
		td = std::thread(monoploy_thread_non_block_run, this);
	}
}

bool CerberusMonopolyThread::loop()
{
	CerberusEvent* event = service->pop_event();
	if (!event)
	{
		return false;
	}

	service->handle_event(event);
	delete event;
    if (service->is_release)
    {
        delete service;
		is_running = false;
        return true;
    }
	return true;
}

bool CerberusMonopolyThread::push_event(CerberusService* service, CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	service->push_event(event);
	active_service_cv.notify_one();
	return true;
}

void CerberusMonopolyThread::join()
{
	td.join();
}

///////////////////////////////////////////////////

CerberusMonopolyThreadMgr::CerberusMonopolyThreadMgr()
{
}

CerberusMonopolyThread *CerberusMonopolyThreadMgr::new_thread(CerberusService *service)
{
	CerberusMonopolyThread* thd = new CerberusMonopolyThread(service);
	monopoly_thread_list.push_back(thd);
	return thd;
}

void CerberusMonopolyThreadMgr::join()
{
	for (auto iter = monopoly_thread_list.begin(); iter != monopoly_thread_list.end(); ++iter)
	{
		(*iter)->join();
	}
}


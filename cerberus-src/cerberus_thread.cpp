
#include "cerberus_thread.h"
#include "cerberus_event.h"
#include "cerberus_service.h"

CerberusThread::CerberusThread()
{
}

CerberusThread::~CerberusThread()
{
}

CerberusShareThread::CerberusShareThread(int thread_num) : thread_num(thread_num)
{
}

void share_thread_run(CerberusShareThread* thread_mgr, int i)
{
	while (true)
	{
		if (!thread_mgr->handle_event())
		{
		    printf("thread sleep i=%d\n", i);
			std::unique_lock<std::mutex> lock(thread_mgr->thread_mtx);
			thread_mgr->active_service_cv.wait(lock, [thread_mgr](){ return !thread_mgr->empty_active_list(); });
		}
	}
}

void CerberusShareThread::dispatch()
{
	std::thread tl[thread_num];
	for (int i = 0; i < thread_num; ++i)
	{
		printf("i=%d\n", i);
		tl[i] = std::thread(share_thread_run, this, i);
	}

	for (int i = 0; i < thread_num; ++i)
	{
		tl[i].join();
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

bool CerberusShareThread::handle_event()
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

void CerberusShareThread::check_active(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	std::unique_lock<std::mutex> lock_small(service->mtx);
	if (service->event_list.empty())
	{
		service->is_active = false;
	}
	else
	{
		active_service_list.push_back(service);
	}
}

bool CerberusShareThread::push_event(CerberusService* service, CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	bool is_already_active = service->push_event(event);
	if (!is_already_active)
	{
		active_service_list.push_back(service);
	}
	active_service_cv.notify_all();
	return true;
}

void CerberusShareThread::add_service(CerberusService* service)
{
	service->thread_mgr = this;
	std::unique_lock<std::mutex> lock_big(thread_mtx);
	active_service_list.push_back(service);
	active_service_cv.notify_all();
}

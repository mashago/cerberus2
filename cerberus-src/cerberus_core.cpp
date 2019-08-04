
extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
}
#include <thread>
#include <condition_variable>

#include "cerberus_util.h"
#include "cerberus_event.h"
#include "cerberus_core.h"
#include "cerberus_service.h"

static std::mutex thread_mtx;
static std::condition_variable active_service_cv;

void monoploy_thread_run(Cerberus* cerberus)
{
}

int Cerberus::dispatch_monopoly_thread_service(CerberusService* service)
{
	// TODO create thread, push service into service list, push start event
	std::thread(monoploy_thread_run, this);
	return 0;
}

// may run in handle_event or in main
// create service, push into service list, push start event
int Cerberus::dispatch_share_thread_service(CerberusService* service)
{
	static int id = 0;
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service->id = ++id;
	service_map.insert(std::make_pair(service->id, service));

	CerberusEvent* start_event = new CerberusEvent();
	start_event->type = CerberusEventType::EVENT_STARTUP;
	start_event->id = 1;

	// raw push event
	service->event_list.push_back(start_event);
	service->is_active = true;

	active_service_list.push_back(service);
	active_service_cv.notify_all();

	return 0;
}

void Cerberus::release_service(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service_map.erase(service->id);
}

bool Cerberus::empty_active_list()
{
    return active_service_list.empty();
}

CerberusService* Cerberus::get_active_service()
{
	CerberusService* service = nullptr;
	std::unique_lock<std::mutex> lock(service_mtx);
	if (!active_service_list.empty())
	{
		service = active_service_list.front();
		active_service_list.pop_front();
	}
	return service;
}

bool Cerberus::push_event(int service_id, CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	auto it = service_map.find(service_id);
	if (it == service_map.end())
	{
		return false;
	}

	CerberusService* service = it->second;
	std::unique_lock<std::mutex> lock_small(service->mtx);
	service->event_list.push_back(event);
	if (!service->is_active)
	{
		service->is_active = true;
		active_service_list.push_back(service);
	}

	active_service_cv.notify_all();
	return true;
}

CerberusEvent* Cerberus::pop_event(CerberusService* service)
{
	CerberusEvent* event = nullptr;
	std::unique_lock<std::mutex> lock(service->mtx);
	if (!service->event_list.empty())
	{
		event = service->event_list.front();
		service->event_list.pop_front();
	}
	return event;
}

bool Cerberus::handle_event()
{
	CerberusService* service = get_active_service();
	if (!service)
	{
		return false;
	}

	CerberusEvent* event = pop_event(service);
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

void Cerberus::check_active(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
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

void share_thread_run(Cerberus* cerberus, int i)
{
	while (true)
	{
		if (!cerberus->handle_event())
		{
		    printf("thread sleep i=%d\n", i);
			std::unique_lock<std::mutex> lock(thread_mtx);
			active_service_cv.wait(lock, [cerberus](){ return !cerberus->empty_active_list(); });
		}
	}
}

void Cerberus::start()
{
	// init main
	CerberusService* s = new TestService(this);
	dispatch_share_thread_service(s);
	
	// init share work threads
	int thread_count = 4;
	std::thread tl[thread_count];
	for (int i = 0; i < thread_count; ++i)
	{
		printf("i=%d\n", i);
		tl[i] = std::thread(share_thread_run, this, i);
	}
	
	for (int i = 0; i < thread_count; ++i)
	{
		tl[i].join();
	}
}

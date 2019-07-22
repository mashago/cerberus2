
extern "C"
{
#include <stdio.h>
#include <unistd.h>
}
#include <thread>
#include "cerberus_core.h"

int Cerberus::create_monopoly_thread_service()
{
	// TODO create thread, create service, push into service list, push start event
	return 0;
}

static int gen_service_id()
{
	static int id = 0;
	return ++id;
}

int Cerberus::create_share_thread_service()
{
	// TODO create service, push into service list, push start event
	CerberusService* service = new CerberusService();
	service->id = gen_service_id();
	service_map.insert(std::make_pair(service->id, service));
	CerberusEvent* start_event = new CerberusEvent();
	start_event->type = 1;
	push_event(service, start_event);

	return 0;
}

CerberusService* Cerberus::get_active_service()
{
	CerberusService* service = nullptr;
	std::unique_lock<std::mutex> lock(active_service_mtx);
	if (!active_service_list.empty())
	{
		service = active_service_list.front();
		active_service_list.pop_front();
	}
	return service;
}

void Cerberus::push_event(CerberusService* service, CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(active_service_mtx);
	std::unique_lock<std::mutex> lock_small(service->mtx);
	service->event_list.push_back(event);
	if (!service->is_active)
	{
		service->is_active = true;
		active_service_list.push_back(service);
	}
	active_service_cv.notify_all();
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
		after_handle_event(service);
		return false;
	}

	// TODO handle event

	after_handle_event(service);
	return true;
}

void Cerberus::after_handle_event(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(active_service_mtx);
	std::unique_lock<std::mutex> lock_small(service->mtx);
	if (service->event_list.empty())
	{
		service->is_active = false;
	}
	else
	{
		service->is_active = true;
		active_service_list.push_back(service);
	}
}

static std::mutex thread_mtx;
void share_thread_run(Cerberus* cerberus)
{
	while (true)
	{
		if (!cerberus->handle_event())
		{
			std::unique_lock<std::mutex> lock(thread_mtx);
			cerberus->active_service_cv.wait(lock, [cerberus](){ return !cerberus->active_service_list.empty(); });
		}
	}
}

void Cerberus::start()
{
	// TODO init share work threads

	int thread_count = 3;

	std::thread tl[thread_count];
	for (int i = 0; i < thread_count; ++i)
	{
		printf("i=%d\n", i);
		tl[i] = std::thread(share_thread_run, this);
	}
	
	for (int i = 0; i < thread_count; ++i)
	{
		tl[i].join();
	}
}

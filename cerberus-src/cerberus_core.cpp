
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

int Cerberus::create_share_thread_service()
{
	// TODO create service, push into service list, push start event
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
	std::unique_lock<std::mutex> lock(service->mtx);
	service->event_list.push_back(event);
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
	if (service == nullptr)
	{
		return false;
	}
	return true;
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

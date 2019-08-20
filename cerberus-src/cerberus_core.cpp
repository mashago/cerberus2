
extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
}
#include <thread>

#include "cerberus_event.h"
#include "cerberus_core.h"
#include "cerberus_service.h"
#include "cerberus_thread.h"

Cerberus::Cerberus() : share_thread_mgr(nullptr)
{
	share_thread_mgr = new CerberusShareThread(4);
}

Cerberus::~Cerberus()
{
	delete share_thread_mgr;
}

static int id = 0;

int Cerberus::dispatch_monopoly_thread_service(CerberusService* service, bool non_block)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service->id = ++id;
	service_map.insert(std::make_pair(service->id, service));

	CerberusEvent* start_event = new CerberusEvent();
	start_event->type = CerberusEventType::EVENT_STARTUP;
	start_event->id = 1;
	service->event_list.push_back(start_event);

	CerberusMonopolyThread* monoploy_thread_mgr = new CerberusMonopolyThread(service, non_block);
	monopoly_thread_list.push_back(monoploy_thread_mgr);
	monoploy_thread_mgr->dispatch();

	return service->id;
}

// may run in handle_event or in main
// create service, push into service list, push start event
int Cerberus::dispatch_share_thread_service(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service->id = ++id;
	service_map.insert(std::make_pair(service->id, service));

	CerberusEvent* start_event = new CerberusEvent();
	start_event->type = CerberusEventType::EVENT_STARTUP;
	start_event->id = 1;

	// raw push event
	service->event_list.push_back(start_event);
	service->is_active = true;
	share_thread_mgr->add_service(service);

	return service->id;
}

void Cerberus::release_service(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service_map.erase(service->id);
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
	service->thread_mgr->push_event(service, event);
	return true;
}

void Cerberus::start()
{
	// init main
	CerberusService* s = new TestService(this);
	dispatch_share_thread_service(s);
	
	share_thread_mgr->dispatch();
}

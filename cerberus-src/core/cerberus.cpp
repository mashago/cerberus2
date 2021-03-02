
extern "C"
{
#include <stdio.h>
#include <stdint.h>
#ifndef WIN32
#include <unistd.h>
#endif
}
#include <thread>

#include "cerberus_event.h"
#include "cerberus.h"
#include "cerberus_service.h"
#include "cerberus_thread.h"
#include "cerberus_loader.h"

Cerberus::Cerberus() : current_service_id(0), share_thread_mgr(nullptr)
{
	share_thread_mgr = new CerberusShareThread();
	monopoly_thread_mgr = new CerberusMonopolyThreadMgr();
	service_loader = new CerberusLoader(this);
}

Cerberus::~Cerberus()
{
	delete share_thread_mgr;
}

int Cerberus::_gen_service_id()
{
	return ++current_service_id;
}

void Cerberus::_add_service(CerberusService *service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service->id = _gen_service_id();
	service_map.insert(std::make_pair(service->id, service));
	service->active();
}

int Cerberus::dispatch_monopoly_thread_service(CerberusService* service)
{
	_add_service(service);
	CerberusMonopolyThread* thd = monopoly_thread_mgr->new_thread(service);
	thd->dispatch();

	return service->id;
}

// may run in handle_event or in main
// create service, push into service list, push start event
int Cerberus::dispatch_share_thread_service(CerberusService* service)
{
	_add_service(service);
	share_thread_mgr->add_service(service);

	return service->id;
}

void Cerberus::release_service(CerberusService* service)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	service_map.erase(service->id);
}

bool Cerberus::push_event(CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_big(service_mtx);
	printf("Cerberus push_event src_id=%d event_type=%d dest_id=%d\n", event->src_id, event->type, event->dest_id);
	auto iter = service_map.find(event->dest_id);
	if (iter == service_map.end())
	{
		return false;
	}

	CerberusService* service = iter->second;
	service->thread_mgr->push_event(service, event);
	return true;
}

void Cerberus::start()
{
    srand((unsigned)time(NULL));
	// init main
	CerberusService *s = service_loader->load("service_test");
    if (!s)
    {
        printf("test load error\n");
        return;
    }
	dispatch_share_thread_service(s);
	
    share_thread_mgr->set_thread_count(4);
	share_thread_mgr->dispatch();
	monopoly_thread_mgr->join();
}

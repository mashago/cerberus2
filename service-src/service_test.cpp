
extern "C"
{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
}

#include <vector>
#include "cerberus_util.h"
#include "cerberus_event.h"
#include "cerberus_core.h"
#include "cerberus_service.h"

#include "service_test.h"

std::vector<int> all_service_vec;

TestService::TestService(Cerberus *c) :
CerberusService(c)
{
}

void TestService::handle_event(CerberusEvent* event)
{
	printf("handle_event src_id=%d event_type=%d dest_id=%d\n", event->src_id, event->type, event->dest_id);

	int service_id = 0;
	int service_count = 8;
	for (int i = 0; i < service_count; ++i)
	{
		TestShareService* s = new TestShareService(c);
		service_id = c->dispatch_share_thread_service(s);
		all_service_vec.push_back(service_id);
	}

	CerberusService* s1 = new TestMolopolyBlockService(c);
	service_id = c->dispatch_monopoly_thread_service(s1);
	all_service_vec.push_back(service_id);

	CerberusService* s2 = new TestMolopolyNonBlockService(c);
	service_id = c->dispatch_monopoly_thread_service(s2);
	all_service_vec.push_back(service_id);
    release();
}

void handle_busy_event()
{
	int n = 0;
	for (int64_t i = 0; i < 100000; i++)
	{
		n += i;
	}
}

CerberusEvent* create_busy_event(int src_id, int dest_id)
{
	CerberusEvent* event = new CerberusEvent();
	event->src_id = src_id;
	event->dest_id = dest_id;
	event->type = CerberusEventType::EVENT_BUSY;
	return event;
}

int random_service(int def)
{
	int size = all_service_vec.size();
	if (size == 0)
	{
		return def;
	}
	return all_service_vec[rand() % size];
}

TestShareService::TestShareService(Cerberus *c) :
CerberusService(c)
{
}

void TestShareService::handle_event(CerberusEvent* event)
{
	printf("TestShareService handle_event src_id=%d event_type=%d dest_id=%d\n", event->src_id, event->type, event->dest_id);
	if (event->type == CerberusEventType::EVENT_BUSY)
	{
		handle_busy_event();
	}

	int dest_id = random_service(id);
	CerberusEvent* new_event = create_busy_event(id, dest_id);
	c->push_event(new_event);
}

TestMolopolyBlockService::TestMolopolyBlockService(Cerberus *c) :
CerberusService(c)
{
}

void TestMolopolyBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyBlockService handle_event src_id=%d event_type=%d dest_id=%d\n", event->src_id, event->type, event->dest_id);
	if (event->type == CerberusEventType::EVENT_BUSY)
	{
		handle_busy_event();
	}

	int dest_id = random_service(id);
	CerberusEvent* new_event = create_busy_event(id, dest_id);
	c->push_event(new_event);
}

TestMolopolyNonBlockService::TestMolopolyNonBlockService(Cerberus *c) :
CerberusService(c)
{
	is_block = false;
}

void TestMolopolyNonBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyNonBlockService handle_event src_id=%d event_type=%d dest_id=%d\n", event->src_id, event->type, event->dest_id);
	if (event->type == CerberusEventType::EVENT_BUSY)
	{
		handle_busy_event();
	}

	int dest_id = random_service(id);
	CerberusEvent* new_event = create_busy_event(id, dest_id);
	c->push_event(new_event);
}

void TestMolopolyNonBlockService::dispatch()
{
	std::list<CerberusEvent*> dest;
	while(true)
	{
		pop_events(dest);
		if (dest.empty())
		{
			// printf("TestMolopolyNonBlockService no event\n");
			// sleep(1);
			continue;
		}
		for (auto iter = dest.begin(); iter != dest.end(); ++iter)
		{
			handle_event(*iter);
			delete *iter;
		}
		dest.clear();
	}
}


#ifdef _WIN32
#define MY_EXPORT __declspec (dllexport)
#else
#define MY_EXPORT
#endif

extern "C" MY_EXPORT void *cerberus_create_service(Cerberus *c)
{
	CerberusService* s = new TestService(c);
    return (void *)s;
}

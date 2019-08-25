
extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
}

#include <vector>
#include "cerberus_util.h"
#include "cerberus_event.h"
#include "cerberus_core.h"
#include "cerberus_service.h"
#include "cerberus_test.h"

std::vector<int> all_service_vec;

TestService::TestService(Cerberus* c) :
CerberusService(c)
{
}

void TestService::handle_event(CerberusEvent* event)
{
	printf("handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	int id = 0;
	int service_count = 4;
	for (int i = 0; i < service_count; ++i)
	{
		TestShareService* s = new TestShareService(c);
		id = c->dispatch_share_thread_service(s);
		all_service_vec.push_back(id);
	}

	CerberusService* s1 = new TestMolopolyBlockService(c);
	id = c->dispatch_monopoly_thread_service(s1);
	all_service_vec.push_back(id);

	CerberusService* s2 = new TestMolopolyNonBlockService(c);
	id = c->dispatch_monopoly_thread_service(s2);
	all_service_vec.push_back(id);
    release();
}

CerberusEvent* create_busy_event(int id, int event_type)
{
	int n = 0;
	for (int64_t i = 0; i < 100000; i++)
	{
		n += i;
	}
	CerberusEvent* event = new CerberusEvent();
	event->id = id;
	event->type = event_type;

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

TestShareService::TestShareService(Cerberus* c) :
CerberusService(c)
{
}

void TestShareService::handle_event(CerberusEvent* event)
{
	printf("TestShareService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(random_service(id), new_event);
}

TestMolopolyBlockService::TestMolopolyBlockService(Cerberus* c) :
CerberusService(c)
{
}

void TestMolopolyBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyBlockService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(random_service(id), new_event);
}

TestMolopolyNonBlockService::TestMolopolyNonBlockService(Cerberus* c) :
CerberusService(c)
{
	is_block = false;
}

void TestMolopolyNonBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyNonBlockService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(random_service(id), new_event);
}

void TestMolopolyNonBlockService::dispatch()
{
	std::list<CerberusEvent*> dest;
	while(true)
	{
		pop_events(dest);
		if (dest.empty())
		{
			printf("TestMolopolyNonBlockService no event\n");
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


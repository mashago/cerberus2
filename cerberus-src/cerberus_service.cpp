
extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
}

#include "cerberus_util.h"
#include "cerberus_event.h"
#include "cerberus_core.h"
#include "cerberus_service.h"

CerberusService::CerberusService(Cerberus* c) :
c(c), is_active(false), is_release(false)
{
}

CerberusService::~CerberusService()
{
}

CerberusEvent* CerberusService::pop_event()
{
	CerberusEvent* event = nullptr;
	std::unique_lock<std::mutex> lock(mtx);
	if (!event_list.empty())
	{
		event = event_list.front();
		event_list.pop_front();
	}
	return event;
}

void CerberusService::pop_events(std::list<CerberusEvent*>& dest)
{
	std::unique_lock<std::mutex> lock(mtx);
	for (auto iter = event_list.begin(); iter != event_list.end(); ++iter)
	{
		dest.push_back(*iter);
	}
	event_list.clear();
}

bool CerberusService::push_event(CerberusEvent* event)
{
	std::unique_lock<std::mutex> lock_small(mtx);
	event_list.push_back(event);
	if (!is_active)
	{
		is_active = true;
		return false;
	}
	return true;
}

void CerberusService::handle_event(CerberusEvent* event)
{
	// default do nothing
	printf("service handle_event do nothing\n");
}

void CerberusService::dispatch()
{
	// default do nothing
	printf("service dispatch do nothing\n");
}

void CerberusService::release()
{
    c->release_service(this);

	// TODO notify event_list call fail to src service

	clear_container(event_list);

    is_release = true;
}

TestService::TestService(Cerberus* c) :
CerberusService(c)
{
}

void TestService::handle_event(CerberusEvent* event)
{
	printf("handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	int service_count = 8;
	for (int i = 0; i < service_count; ++i)
	{
		TestShareService* s = new TestShareService(c);
		c->dispatch_share_thread_service(s);
	}
    release();
}

TestShareService::TestShareService(Cerberus* c) :
CerberusService(c)
{
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

void TestShareService::handle_event(CerberusEvent* event)
{
	printf("TestShareService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(id, new_event);
}

TestMolopolyBlockService::TestMolopolyBlockService(Cerberus* c) :
CerberusService(c)
{
}

void TestMolopolyBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyBlockService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(id, new_event);
}

TestMolopolyNonBlockService::TestMolopolyNonBlockService(Cerberus* c) :
CerberusService(c)
{
}

void TestMolopolyNonBlockService::handle_event(CerberusEvent* event)
{
	printf("TestMolopolyNonBlockService handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	CerberusEvent* new_event = create_busy_event(event->id + 1, CerberusEventType::EVENT_CUSTOM);
	c->push_event(id, new_event);
}

void TestMolopolyNonBlockService::dispatch()
{
	std::list<CerberusEvent*> dest;
	while(true)
	{
		pop_events(dest);
		for (auto iter = dest.begin(); iter != dest.end(); ++iter)
		{
			handle_event(*iter);
			delete *iter;
		}
		dest.clear();
	}
}


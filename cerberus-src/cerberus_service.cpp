
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

void CerberusService::handle_event(CerberusEvent* event)
{
	// default do nothing
	printf("service do nothing\n");
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
		TestSubService* s = new TestSubService(c);
		c->dispatch_share_thread_service(s);
	}
    release();
}

TestSubService::TestSubService(Cerberus* c) :
CerberusService(c)
{
}

void TestSubService::handle_event(CerberusEvent* event)
{
	printf("handle_event service_id=%d event_type=%d event_id=%d\n", id, event->type, event->id);

	// fake handle event and test create new event
	int n = 0;
	for (int64_t i = 0; i < 100000; i++)
	{
		n += i;
	}
	CerberusEvent* new_event = new CerberusEvent();
	new_event->type = CerberusEventType::EVENT_CUSTOM;
	new_event->id = event->id + 1;

	c->push_event(id, new_event);
}

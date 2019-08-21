
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


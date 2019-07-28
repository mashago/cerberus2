#pragma once

#include <list>
#include <mutex>

class Cerberus;

struct CerberusEvent
{
	int type;
	int id;
};

class CerberusService
{
public:
	int id;
	Cerberus* c;
	bool is_active;
	CerberusService(Cerberus* c);
	std::mutex mtx;
	std::list<CerberusEvent*> event_list;
	virtual void handle_event(CerberusEvent* event);
};

class TestService : public CerberusService
{
public:
	TestService(Cerberus* c);
	void handle_event(CerberusEvent* event);
};

class TestSubService : public CerberusService
{
public:
	TestSubService(Cerberus* c);
	void handle_event(CerberusEvent* event);
};

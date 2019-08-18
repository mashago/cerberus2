#pragma once

#include <list>
#include <mutex>

class Cerberus;
class CerberusEvent;
class CerberusThread;

class CerberusService
{
public:
	int id;
	Cerberus* c;
	bool is_active;
    bool is_release;
	std::mutex mtx;
	std::list<CerberusEvent*> event_list;
	CerberusThread* thread_mgr;

	CerberusService(Cerberus* c);
    virtual ~CerberusService();
	
	CerberusEvent* pop_event();
	bool push_event(CerberusEvent* event);
	virtual void handle_event(CerberusEvent* event);
	virtual void dispatch();
    void release();
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

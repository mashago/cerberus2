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
	bool is_block; // work with monopoly thread
	std::mutex mtx;
	std::list<CerberusEvent*> event_list;
	CerberusThread* thread_mgr;

	CerberusService();
	CerberusService(Cerberus* c);
    virtual ~CerberusService();
	
    void set_cerberus(Cerberus *c);
	CerberusEvent* pop_event();
	void pop_events(std::list<CerberusEvent*>& l);
	bool push_event(CerberusEvent* event);
	virtual void handle_event(CerberusEvent* event);
	virtual void dispatch(); // work with monopoly nonblock thread
    void release();
};

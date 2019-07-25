
#pragma once

#include <list>
#include <map>
#include <mutex>
#include <condition_variable>

class Cerberus;

struct CerberusEvent
{
	int type;
	int id;
};

struct CerberusService
{
	int id;
	bool is_active;
	std::mutex mtx;
	std::list<CerberusEvent*> event_list;
	void handle_event(CerberusEvent* event);
	Cerberus* c;
};

class Cerberus
{
public:
	std::list<CerberusService*> active_service_list;
	int create_monopoly_thread_service();
	int create_share_thread_service();

	void push_event(CerberusService* service, CerberusEvent* event);
	CerberusEvent* pop_event(CerberusService* service);
	bool handle_event();
	void start();
	std::condition_variable active_service_cv;
private:
	std::mutex active_service_mtx;
	std::map<int, CerberusService*> service_map;

	CerberusService* get_active_service();
	void after_handle_event(CerberusService* service);
};

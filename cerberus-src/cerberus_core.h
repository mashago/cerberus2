#pragma once

#include <list>
#include <map>
#include <mutex>

class CerberusEvent;
class CerberusService;

class Cerberus
{
public:
	int dispatch_monopoly_thread_service(CerberusService* service);
	int dispatch_share_thread_service(CerberusService* service);
	void release_service(CerberusService* service);
    bool empty_active_list();

	bool push_event(int service_id, CerberusEvent* event);
	CerberusEvent* pop_event(CerberusService* service);
	bool handle_event();
	void start();
private:
	std::mutex service_mtx;
	std::map<int, CerberusService*> service_map;
	std::list<CerberusService*> active_service_list;

	CerberusService* get_active_service();
	void check_active(CerberusService* service);
};

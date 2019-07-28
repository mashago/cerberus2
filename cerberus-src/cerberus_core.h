#pragma once

#include <list>
#include <map>
#include <mutex>
#include <condition_variable>

class CerberusEvent;
class CerberusService;

class Cerberus
{
public:
	std::list<CerberusService*> active_service_list;
	int create_monopoly_thread_service();
	int dispatch_share_thread_service(CerberusService* service);
	void release_service(int service_id);

	bool push_event(int service_id, CerberusEvent* event);
	CerberusEvent* pop_event(CerberusService* service);
	bool handle_event();
	void start();
	std::condition_variable active_service_cv;
private:
	std::mutex service_mtx;
	std::map<int, CerberusService*> service_map;

	CerberusService* get_active_service();
	void check_active(CerberusService* service);
};

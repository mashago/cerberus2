
#pragma once

#include <list>
#include <map>
#include <mutex>
#include <condition_variable>

void cerberus_start();

struct CerberusEvent {
};

struct CerberusService {
	int id;
	std::mutex mtx;
	std::list<CerberusEvent> event_list;
};

class Cerberus {
public:
	std::list<CerberusService*> active_service_list;
	int create_monopoly_thread_service();
	int create_share_thread_service();
	bool handle_event();
	void start();
private:
	std::mutex mtx;
	std::map<int, CerberusService*> service_map;

	CerberusService* get_active_service();
};

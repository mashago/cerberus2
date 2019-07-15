
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
	std::list<CerberusEvent> event_list;
};

class Cerberus {
public:
	int cerberus_create_monopoly_thread_service();
	int cerberus_create_share_thread_service();
private:
	std::mutex mt;
	std::condition_variable cv;
	std::map<int, CerberusService*> service_map;
};

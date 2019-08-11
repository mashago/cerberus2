#pragma once

#include <list>
#include <map>
#include <mutex>

class CerberusEvent;
class CerberusService;
class CerberusShareThread;
class CerberusMonopolyThread;

class Cerberus
{
public:
	Cerberus();
	~Cerberus();
	int dispatch_monopoly_thread_service(CerberusService* service);
	int dispatch_share_thread_service(CerberusService* service);
	void release_service(CerberusService* service);
	bool push_event(int service_id, CerberusEvent* event);
	void start();
private:
	std::mutex service_mtx;
	std::map<int, CerberusService*> service_map;
	CerberusShareThread* share_thread_mgr;
	std::list <CerberusMonopolyThread*> monopoly_thread_list;
};

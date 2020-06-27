#pragma once

#include <list>
#include <map>
#include <mutex>

class CerberusEvent;
class CerberusService;
class CerberusShareThread;
class CerberusMonopolyThreadMgr;
class CerberusLoader;

class Cerberus
{
public:
	Cerberus();
	~Cerberus();
	int dispatch_monopoly_thread_service(CerberusService *service);
	int dispatch_share_thread_service(CerberusService *service);
	void release_service(CerberusService *service);
	bool push_event(CerberusEvent *event);
	void start();
private:
	int current_service_id;
	std::mutex service_mtx;
	std::map<int, CerberusService *> service_map;
	CerberusShareThread *share_thread_mgr;
	CerberusMonopolyThreadMgr *monopoly_thread_mgr;
    CerberusLoader *service_loader;
	int _gen_service_id();
	void _add_service(CerberusService *service);
};

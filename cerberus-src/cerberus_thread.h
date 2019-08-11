#pragma once

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

class CerberusEvent;
class CerberusService;

class CerberusThread
{
public:
	CerberusThread();
	virtual ~CerberusThread();
	virtual void dispatch() = 0;
	virtual void add_service(CerberusService* service) = 0;
	virtual bool push_event(CerberusService* service, CerberusEvent* event) = 0;
	std::condition_variable active_service_cv;
};

class CerberusShareThread : public CerberusThread
{
public:
	CerberusShareThread(int thread_num);
	void dispatch();
	void add_service(CerberusService* service);
	bool push_event(CerberusService* service, CerberusEvent* event);
	void check_active(CerberusService* service);
	bool handle_event();
	bool empty_active_list();
	CerberusService* get_active_service();
	std::mutex thread_mtx;
private:
	const int thread_num;
	std::list<CerberusService*> active_service_list;
};

class CerberusMonopolyThread
{
public:
	CerberusMonopolyThread(CerberusService* service);
	void add_service(CerberusService* service);
	bool handle_event();
private:
	CerberusService* service;
};

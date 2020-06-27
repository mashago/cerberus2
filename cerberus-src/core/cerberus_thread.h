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
	virtual bool loop() = 0;
	virtual bool push_event(CerberusService* service, CerberusEvent* event) = 0;

	bool is_running;
	std::mutex thread_mtx;
	std::condition_variable active_service_cv;
};

class CerberusShareThread : public CerberusThread
{
public:
	CerberusShareThread();
	void dispatch();
	bool loop();
	bool push_event(CerberusService* service, CerberusEvent* event);

	void add_service(CerberusService* service);
	void check_active(CerberusService* service);
	bool empty_active_list();
	CerberusService* get_active_service();
private:
	std::list<CerberusService*> active_service_list;
};

class CerberusMonopolyThread : public CerberusThread
{
public:
	CerberusMonopolyThread(CerberusService* service);
	void dispatch();
	bool loop();
	bool push_event(CerberusService* service, CerberusEvent* event);

	void join();
	CerberusService* service;
private:
	std::thread td;
};

class CerberusMonopolyThreadMgr
{
public:
	CerberusMonopolyThreadMgr();
	CerberusMonopolyThread *new_thread(CerberusService *service);
	void join();
private:
	std::list <CerberusMonopolyThread *> monopoly_thread_list;
};


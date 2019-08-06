#pragma once

#include <mutex>
#include <condition_variable>

class Cerberus;

class CerberusThread
{
public:
	virtual ~CerberusThread();
	virtual void dispatch();
	virtual void notify();
	virtual bool push_event(CerberusService* service, CerberusEvent* event);
	virtual CerberusEvent* pop_event(CerberusService* service);
private:
	std::condition_variable active_service_cv;
};

class CerberusShareThread
{
public:
	CerberusShareThread(Cerberus* c, int num);
	void notify();
private:
	std::mutex thread_mtx;
	std::list<CerberusService*> active_service_list;
	bool handle_event();
};

class CerberusMonopolyThread
{
public:
	CerberusMonopolyThread(Cerberus* c, CerberusService* service);
	void notify();
private:
	CerberusService* service;
	bool handle_event();
};

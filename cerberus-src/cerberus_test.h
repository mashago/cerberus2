#pragma once

class CerberusService;
class CerberusEvent;

class TestService : public CerberusService
{
public:
	TestService(Cerberus* c);
	void handle_event(CerberusEvent* event);
};

class TestShareService : public CerberusService
{
public:
	TestShareService(Cerberus* c);
	void handle_event(CerberusEvent* event);
};

class TestMolopolyBlockService : public CerberusService
{
public:
	TestMolopolyBlockService(Cerberus* c);
	void handle_event(CerberusEvent* event);
};

class TestMolopolyNonBlockService : public CerberusService
{
public:
	TestMolopolyNonBlockService(Cerberus* c);
	void dispatch();
	void handle_event(CerberusEvent* event);
};


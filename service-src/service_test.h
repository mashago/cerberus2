#pragma once

class CerberusService;
class CerberusEvent;

class TestService : public CerberusService
{
public:
	TestService();
	void handle_event(CerberusEvent* event);
};

class TestShareService : public CerberusService
{
public:
	TestShareService();
	void handle_event(CerberusEvent* event);
};

class TestMolopolyBlockService : public CerberusService
{
public:
	TestMolopolyBlockService();
	void handle_event(CerberusEvent* event);
};

class TestMolopolyNonBlockService : public CerberusService
{
public:
	TestMolopolyNonBlockService();
	void dispatch();
	void handle_event(CerberusEvent* event);
};


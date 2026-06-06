#pragma once

class Cerberus;
class CerberusService;
class CerberusEvent;

class TestService : public CerberusService
{
public:
	TestService(Cerberus *c);
	void handle_event(CerberusEvent* event);
};

class TestShareService : public CerberusService
{
public:
	TestShareService(Cerberus *c);
	void handle_event(CerberusEvent* event);
};

class TestMonopolyBlockService : public CerberusService
{
public:
	TestMonopolyBlockService(Cerberus *c);
	void handle_event(CerberusEvent* event);
};

class TestMonopolyNonBlockService : public CerberusService
{
public:
	TestMonopolyNonBlockService(Cerberus *c);
	void dispatch();
	void handle_event(CerberusEvent* event);
};


#pragma once

enum CerberusEventType {
    EVENT_STARTUP = 1,
    EVENT_CUSTOM = 99,
};

class CerberusEvent
{
public:
	int type;
	int id;
};


#pragma once

enum CerberusEventType {
    EVENT_STARTUP = 1,
    EVENT_CUSTOM = 99,
};

struct CerberusEvent
{
	int type;
	int id;
};


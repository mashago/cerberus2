#pragma once

enum CerberusEventType {
    EVENT_STARTUP = 1,
    EVENT_CUSTOM = 99,
    EVENT_BUSY = 100,
};

class CerberusEvent
{
public:
	int type;
	int id;

	int src_id;
	int dest_id;
	int session_id;
};


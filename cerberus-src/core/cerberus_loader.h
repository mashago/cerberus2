#pragma once

#include <map>
#include <mutex>
#include "cerberus_util.h"

class Cerberus;
class CerberusService;
class CerberusLoader
{
public:
	Cerberus* c;
    std::map<std::string, void *> dl_map;
    std::mutex mtx;

	CerberusLoader(Cerberus* c);
    CerberusService *load(const char *service_name);    
};

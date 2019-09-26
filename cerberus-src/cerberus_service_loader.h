#pragma once

#include <map>
#include <mutex>
#include "cerberus_util.h"

class CerberusService;
class CerberusServiceLoader
{
public:
    CerberusService *load(const char *service_name);    
    void unload(const char *service_name);
    std::map<std::string, void *> dl_map;
    std::mutex mtx;
};

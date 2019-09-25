#pragma once

#include <map>
#include "cerberus_util.h"

class CerberusService;
class CerberusServiceLoader
{
public:
    CerberusService *load(const char *service_name);    
    void unload(const char *service_name);
    std::map<std::string, dl_func> open_func_map; // lib_name to dl_func
};

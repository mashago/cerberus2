
#include <stdio.h>
#include "cerberus_util.h"
#include "cerberus_service_loader.h"

static const int BUFFER_SIZE = 100;
static char error_msg[256];
static const char *DL_OPEN_FUNC = "cerberus_open_service";

CerberusService *CerberusServiceLoader::load(const char *service_name)
{
    // TODO get lib name
    dl_func open_func = nullptr;
    auto iter = open_func_map.find(service_name);
    if (iter != open_func_map.end())
    {
        open_func = iter->second;
    }
    else
    {
        void *lib = dl_load_lib(service_name);
        if (!lib)
        {
            printf("dl_load_lib fail %s\n", dl_error(error_msg, BUFFER_SIZE));
            return nullptr;
        }

        open_func = dl_load_func(lib, DL_OPEN_FUNC);
        if (!open_func)
        {
            printf("dl_load_func fail %s\n", dl_error(error_msg, BUFFER_SIZE));
            dl_unload_lib(lib);
            return nullptr;
        }
        open_func_map.insert(std::make_pair(service_name, open_func));
    }

    CerberusService *service = (CerberusService *)open_func();
    return service;
}

void CerberusServiceLoader::unload(const char *service_name)
{
    open_func_map.erase(service_name);
}

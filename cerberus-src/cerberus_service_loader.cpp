
#include <stdio.h>
#include "cerberus_util.h"
#include "cerberus_service_loader.h"

static const char *DL_OPEN_FUNC = "cerberus_open_service";

const char *_get_dl_name(const char *service_name)
{
    static const int PATH_SIZE = 512;
    static char path[PATH_SIZE];

#if __APPLE__
    snprintf(path, PATH_SIZE, "lib%s.dylib", service_name);
#elif WIN32
    snprintf(path, PATH_SIZE, "%s.dll", service_name);
#else
    snprintf(path, PATH_SIZE, "lib%s.so", service_name);
#endif

    return path;
}

CerberusService *CerberusServiceLoader::load(const char *service_name)
{
	std::unique_lock<std::mutex> lock(mtx);
    const int BUFFER_SIZE = 100;
    char error_msg[BUFFER_SIZE];
    const char *path = _get_dl_name(service_name);

    void *lib = nullptr;
    auto iter = dl_map.find(path);
    if (iter != dl_map.end())
    {
        lib = iter->second;
    }
    else
    {
        lib = dl_load_lib(path);
        if (!lib)
        {
            printf("dl_load_lib fail %s\n", dl_error(error_msg, BUFFER_SIZE));
            return nullptr;
        }

        dl_map.insert(std::make_pair(path, lib));
    }

    dl_func open_func = dl_load_func(lib, DL_OPEN_FUNC);
    if (!open_func)
    {
        printf("dl_load_func fail %s\n", dl_error(error_msg, BUFFER_SIZE));
        dl_unload_lib(lib);
        return nullptr;
    }

    CerberusService *service = (CerberusService *)open_func();
    return service;
}

void CerberusServiceLoader::unload(const char *service_name)
{
    const char *path = _get_dl_name(service_name);
    dl_map.erase(path);
}

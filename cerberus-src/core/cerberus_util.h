#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <memory>

// clear pointer container
template <typename TP, template <typename E, typename Alloc = std::allocator<E>> class TC>
void clear_container(TC<TP> &c)
{
    while (!c.empty())
    {
        auto iter = c.begin();
        delete *iter;
        *iter = nullptr;
        c.erase(iter);
    }
}


void *dl_load_lib(const char *path);
void *dl_load_func(void *lib, const char *sym);
void dl_unload_lib(void *lib);
char *dl_error(char *buffer, int size);

#ifdef WIN32

#define snprintf(buffer, count, format, ...) do {_snprintf_s(buffer, count, count-1, format, ##__VA_ARGS__);} while (false)

inline void sleep(int second)
{
	Sleep(second * 1000);
}

#endif

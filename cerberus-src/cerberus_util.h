#pragma once

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

typedef void* (*dl_func)(void);

void *dl_load_lib(const char *path);
dl_func dl_load_func(void *lib, const char *sym);
void dl_unload_lib(void *lib);
char *dl_error(char *buffer, int size);

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

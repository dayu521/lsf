module;
#include <string>
// #include <concepts>

module lsf:parser_tree.imp;

import :parser_tree;
import :tree_allocator;

namespace lsf
{

    void *new_bridge(std::size_t count)
    {
        if (count > Inner::MyAllocator::MaxObjSize)
            return ::operator new(count);
        auto p = Inner::get_singleton<Inner::MyAllocator>().allocate(count);
        if (p == nullptr)
            //        throw std::bad_alloc("MyAllocator allocate failed");
            throw std::bad_alloc();
        return p;
    }
    void delete_bridge(void *ptr, std::size_t sz)
    {
        if (sz > Inner::MyAllocator::MaxObjSize)
            ::operator delete(ptr);
        else
        {
            Inner::get_singleton<Inner::MyAllocator>().deallocate(ptr, sz);
        }
    }


} // namespace lsf

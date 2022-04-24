#include "reio/allocators.hpp"

namespace reio
{

    byte* default_allocator::allocate(std::size_t size)
    {
        return ::new byte[size];
    }

    void default_allocator::deallocate(byte *ptr)
    {
        ::delete [] ptr;
    }

}

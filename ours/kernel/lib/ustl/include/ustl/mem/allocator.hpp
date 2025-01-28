/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///

#ifndef USTL_MEM_ALLOCATOR_HPP
#define USTL_MEM_ALLOCATOR_HPP 1

#include <ustl/config.hpp>

namespace ustl::mem {
    template <typename T>
    struct Allocator
    {
        typedef T const *   Ptr;
        typedef T const &   Ref;
        typedef T *         PtrMut;
        typedef T &         RefMut;
        typedef T           Element;

        auto allocate(usize n) -> T *
        {  return operator new(n * sizeof(T));  }

        auto deallocate(PtrMut ptr) -> void
        {  return operator delete(ptr);  }

        auto deallocate(PtrMut ptr, usize n) -> void
        {  return operator delete[](ptr, n);  }
    };

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_ALLOCATOR_HPP
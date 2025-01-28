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

#ifndef USTL_ALLOC_DUMMY_ALLOCATOR_HPP
#define USTL_ALLOC_DUMMY_ALLOCATOR_HPP 1

#include <ustl/config.hpp>

namespace ustl::alloc {
    class DummyAllocator
    {
        typedef void const *   Ptr;
        typedef void const &   Ref;
        typedef void *         PtrMut;
        typedef void &         RefMut;
        typedef void           Element;
    public:
        auto allocate(usize n) -> void *
        {  return nullptr;  }

        auto deallocate(void *ptr, usize n) -> void
        {}
    };
} // namespace ustl::alloc

#endif // #ifndef USTL_ALLOC_DUMMY_ALLOCATOR_HPP
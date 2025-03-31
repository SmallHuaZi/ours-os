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

#ifndef USTL_MEM_ALLOCATOR_TRAITS_HPP
#define USTL_MEM_ALLOCATOR_TRAITS_HPP 1

#include <ustl/config.hpp>
#include <ustl/limits.hpp>

namespace ustl::mem {
    template <typename T>
    struct AllocatorTraits
    {
        typedef T        Allocator;
        typedef typename Allocator::Ptr         Ptr;
        typedef typename Allocator::Ref         Ref;
        typedef typename Allocator::PtrMut      PtrMut;
        typedef typename Allocator::RefMut      RefMut;
        typedef typename Allocator::Element     Element;

        USTL_FORCEINLINE
        static auto allocate(Allocator &allocator, usize n = 1) -> T *
        {  return allocator.allocate(n);  }

        USTL_FORCEINLINE
        static auto deallocate(Allocator &allocator, PtrMut ptr, usize n = 1) -> void 
        {  allocator.deallocate(ptr, n);  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        static auto max_size() -> usize
        {  return NumericLimits<usize>::max();  }
    };

} // namespace ustl::alloc

#endif // #ifndef USTL_ALLOC_ALLOCATOR_TRAITS_HPP
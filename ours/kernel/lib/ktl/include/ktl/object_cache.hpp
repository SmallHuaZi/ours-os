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

#ifndef KTL_OBJECT_CACHE_HPP
#define KTL_OBJECT_CACHE_HPP 1

namespace ktl {
    template <typename T, typename A, int CacheBlockSize>
    struct ObjectCache
    {
        typedef T                   Element;
        typedef Element *           PtrMut;
        typedef Element &           RefMut;
        typedef Element const *     Ptr;
        typedef Element const &     Ref;

        template <typename... Args> 
        auto allocate(Args&&... args) -> T *
        {}

        auto release(Ptr ptr) -> void
        {}
    };
} // namespace ktl

#endif // #ifndef KTL_OBJECT_CACHE_HPP
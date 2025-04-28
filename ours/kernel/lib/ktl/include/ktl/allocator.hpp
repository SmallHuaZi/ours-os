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
#ifndef KTL_ALLOCATOR_HPP
#define KTL_ALLOCATOR_HPP 1

#include <ktl/new.hpp>

namespace ktl {
    template <typename T>
    struct Allocator {
        typedef T   Element;
        typedef Element *        PtrMut;
        typedef Element const *  Ptr;
        typedef Element &        RefMut;
        typedef Element const &  Ref;

        typedef Element value_type;
        typedef PtrMut  pointer;
        typedef Ptr     const_pointer;
        typedef RefMut  reference;
        typedef Ref     const_reference;

        template <typename U>
        using RebindT = Allocator<U>;

        static auto allocate(usize n) -> PtrMut {
            return new (ours::mem::kGafKernel) Element[n]();
        }

        static auto deallocate(Ptr ptr, usize n) -> void {
            delete[] ptr;
        }
    };

} // namespace ktl

#endif // #ifndef KTL_ALLOCATOR_HPP
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

#ifndef USTL_MEM_VOIDIFY_HPP
#define USTL_MEM_VOIDIFY_HPP 1

#include <ustl/mem/address_of.hpp>

namespace ustl::mem {
    template <typename T>
    USTL_FORCEINLINE
    auto voidify(T& t) USTL_NOEXCEPT -> void *
    {
        // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
        return const_cast<void*>
        (
            static_cast<const volatile void*>
            (
                mem::address_of(t)
            )
        );
    }

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_VOIDIFY_HPP
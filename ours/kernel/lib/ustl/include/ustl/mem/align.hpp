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

#ifndef USTL_MEM_ALIGN_HPP
#define USTL_MEM_ALIGN_HPP 1

#include <ustl/config.hpp>

namespace ustl::mem {
    USTL_FORCEINLINE USTL_CONSTEXPR  
    auto align_down(usize n, usize a) USTL_NOEXCEPT -> usize 
    { return n & ~(a - 1); }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto align_up(usize n, usize a) USTL_NOEXCEPT -> usize
    { return (n + a - 1) & ~(a - 1); }

    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto is_aligned(usize n, usize a) USTL_NOEXCEPT -> bool 
    { return (n & (a - 1)) == 0; }

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_ALIGN_HPP
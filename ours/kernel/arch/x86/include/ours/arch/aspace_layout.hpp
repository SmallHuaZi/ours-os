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

#ifndef OURS_ARCH_ASPACE_LAYOUT_HPP
#define OURS_ARCH_ASPACE_LAYOUT_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#ifndef OURS_CONFIG_KERNEL_ASPACE_BASE 
#define OURS_CONFIG_KERNEL_ASPACE_BASE 0xffffff8000000000UL
#endif // #ifndef OURS_CONFIG_KERNEL_ASPACE_BASE

#ifndef OURS_CONFIG_KERNEL_ASPACE_SIZE
#define OURS_CONFIG_KERNEL_ASPACE_SIZE 0x0000008000000000UL
#endif // #ifndef OURS_CONFIG_KERNEL_ASPACE_SIZE

#ifndef OURS_CONFIG_USER_ASPACE_BASE
#define OURS_CONFIG_USER_ASPACE_BASE 0xffffff8000000000UL
#endif // #ifndef OURS_CONFIG_USER_ASPACE_BASE 

#ifndef OURS_CONFIG_USER_ASPACE_SIZE
#define OURS_CONFIG_USER_ASPACE_SIZE 0x0000008000000000UL
#endif // #ifndef OURS_CONFIG_USER_ASPACE_SIZE 

namespace ours::mem {
    CXX11_CONSTEXPR
    static usize const KERNEL_ASPACE_BASE = OURS_CONFIG_KERNEL_ASPACE_BASE;

    CXX11_CONSTEXPR
    static usize const KERNEL_ASPACE_SIZE = OURS_CONFIG_KERNEL_ASPACE_SIZE;

    CXX11_CONSTEXPR
    static usize const USER_ASPACE_BASE = OURS_CONFIG_USER_ASPACE_BASE;

    CXX11_CONSTEXPR
    static usize const USER_ASPACE_SIZE = OURS_CONFIG_USER_ASPACE_SIZE;

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_ASPACE_LAYOUT_HPP
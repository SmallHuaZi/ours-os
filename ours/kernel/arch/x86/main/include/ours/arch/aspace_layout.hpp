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

#include <ours/macro_abi.hpp>

#define KERNEL_ASPACE_BASE  0xffffff8000000000UL
#define KERNEL_ASPACE_SIZE  GB(512)

#define ARCH_PHYSMAP_PHYS_BASE  0 
#define ARCH_PHYSMAP_VIRT_BASE  KERNEL_ASPACE_BASE
#define ARCH_PHYSMAP_SIZE       GB(32) 

#define USER_ASPACE_BASE    0xffffff8000000000UL
#define USER_ASPACE_SIZE    0x0000008000000000UL

#endif // #ifndef OURS_ARCH_ASPACE_LAYOUT_HPP 
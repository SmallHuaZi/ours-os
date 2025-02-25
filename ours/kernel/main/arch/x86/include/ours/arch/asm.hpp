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

#ifndef OURS_ARCH_ASM_HPP
#define OURS_ARCH_ASM_HPP 1

#define KERNEL_LOAD_ADDR    0x0000000000100000
#define KERNEL_BASE         0xffffff8000100000

#define PHYS(x) ((x) - KERNEL_BASE + KERNEL_LOAD_ADDR)

#endif // #ifndef OURS_ARCH_ASM_HPP
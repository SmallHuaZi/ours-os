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

#ifndef OURS_ARCH_X86_CONSTANT_HPP
#define OURS_ARCH_X86_CONSTANT_HPP 1

#define X86_GDT_KERNEL_CODE64    (1 << 3)
#define X86_GDT_KERNEL_CODE32    (2 << 3) 
#define X86_GDT_KERNEL_DATA      (3 << 3) 
#define X86_GDT_USER_CODE64      (4 << 3) 
#define X86_GDT_USER_CODE32      (5 << 3)
#define X86_GDT_USER_DATA        (6 << 3)

#endif // #ifndef OURS_ARCH_X86_CONSTANT_HPP
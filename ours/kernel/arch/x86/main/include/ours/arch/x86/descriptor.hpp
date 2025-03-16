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

#ifndef OURS_ARCH_X86_DESCRIPTOR_HPP
#define OURS_ARCH_X86_DESCRIPTOR_HPP 1

/// GDT selectors
#define X86_GDT_KERNEL_CODE32    (1 << 3)
#define X86_GDT_KERNEL_DATA      (2 << 3)
#define X86_GDT_KERNEL_CODE64    (3 << 3)
#define X86_GDT_USER_CODE32      (4 << 3)
#define X86_GDT_USER_CODE64      (5 << 3)
#define X86_GDT_USER_DATA        (6 << 3)

#define TSS_SELECTOR(i) ((uint16_t)(0x38 + 16 * (i)))
/* 0x40 is used by the second half of the first TSS descriptor */

/// selector priviledge level
#define SELECTOR_PL(s) ((s)&0x3)

/// Descriptor types
#define SEG_TYPE_TSS        0x9
#define SEG_TYPE_TSS_BUSY   0xb
#define SEG_TYPE_TASK_GATE  0x5
#define SEG_TYPE_INT_GATE   0xe /* 32 bit */
#define SEG_TYPE_DATA_RW    0x2
#define SEG_TYPE_CODE_RW    0xa

#endif // #ifndef OURS_ARCH_X86_DESCRIPTOR_HPP
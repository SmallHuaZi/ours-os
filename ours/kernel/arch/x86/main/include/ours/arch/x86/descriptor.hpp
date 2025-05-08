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

#define X86_GDT_KERNEL_CODE32    (1 << 3)
#define X86_GDT_KERNEL_CODE64    (2 << 3)
#define X86_GDT_KERNEL_DATA      (3 << 3)

#define X86_GDT_USER_CODE32      (4 << 3)
#define X86_GDT_USER_DATA        (5 << 3)
#define X86_GDT_USER_CODE64      (6 << 3)
#define X86_GDT_MAX_SELECTORS    (6 + 1) // Has also the first null descriptor

#define TSS_SELECTOR(i) ((u16)(0x38 + 16 * (i)))
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

#ifndef __ASSEMBLY__

#include <ours/config.hpp>
#include <ours/cpu-cfg.hpp>
#include <ours/mem/cfg.hpp>

#include <arch/x86/descriptor.hpp>
#include <ustl/array.hpp>

namespace ours {
    auto x86_load_gdt() -> void;

    auto x86_dump_gdt() -> void;

    auto x86_set_tss_sp(VirtAddr sp) -> void;

} // namespace ours
#endif // #ifndef __ASSEMBLY__

#endif // #ifndef OURS_ARCH_X86_DESCRIPTOR_HPP
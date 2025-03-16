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
#ifndef ARCH_MACRO_MMU_FLAGS_HPP
#define ARCH_MACRO_MMU_FLAGS_HPP 1

#include <ours/macro_abi.hpp>

#define X86_MMUF_PRESENT            BIT(0)
#define X86_MMUF_WRITABLE           BIT(1)
#define X86_MMUF_USER               BIT(2)
#define X86_MMUF_WRITE_THROUGH      BIT(3)
#define X86_MMUF_DISCACHE           BIT(4)
#define X86_MMUF_ACCESSED           BIT(5)
#define X86_MMUF_DIRTY              BIT(6)
#define X86_MMUF_PAGE_SIZE          BIT(7)
#define X86_MMUF_GLOBAL             BIT(8)
#define X86_MMUF_NOEXCUTABLE        BIT(63) // Active only on x86_64

#define X86_MMF_PERM_MASK \
    (X86_MMUF_PRESENT | X86_MMUF_ACCESSED | X86_MMUF_WRITABLE)

/// Default flags for 4KB page directory entries.
#define X86_KERNEL_PD_FLAGS \
    (X86_MMUF_PRESENT | X86_MMUF_ACCESSED | X86_MMUF_WRITABLE | \
     X86_MMUF_GLOBAL)

/// Default flags for 2MB/4MB/1GB page directory entries.
#define X86_KERNEL_HUGE_PAGE_FLAGS \
    (X86_MMUF_PRESENT | X86_MMUF_ACCESSED | X86_MMUF_WRITABLE | \
     X86_MMUF_GLOBAL | X86_MMUF_PAGE_SIZE | X86_MMUF_DIRTY)

/// Default flags for 4KB page.
#define X86_KERNEL_PAGE_FLAGS \
    (X86_MMUF_PRESENT | X86_MMUF_ACCESSED | X86_MMUF_WRITABLE | \
     X86_MMUF_GLOBAL | X86_MMUF_DIRTY)

#define X86_PML5_SHIFT   47
#define X86_PML4_SHIFT   39
#define X86_PDP_SHIFT    30
#define X86_PD_SHIFT     21
#define X86_PT_SHIFT     12

#define X86_PAGE_SHIFT   12
#define X86_PFN_MASK     (~(BIT(12)-1))

#define X86_MAX_PTES    512

/* page fault error code flags */
#define PFEX_PRESENT    BIT(0)
#define PFEX_WRITE      BIT(1)
#define PFEX_USER       BIT(2)
#define PFEX_RSV        BIT(3)
#define PFEX_I          BIT(4)
#define PFEX_PK         BIT(5)
#define PFEX_SGX        BIT(15)

#define PFEX_MASK \
       (PFEX_PRESENT | PFEX_WRITE | PFEX_USER | PFEX_RSV | PFEX_I |\
        PFEX_PK | PFEX_SGX)

#endif // #ifndef ARCH_MACRO_MMU_FLAGS_HPP
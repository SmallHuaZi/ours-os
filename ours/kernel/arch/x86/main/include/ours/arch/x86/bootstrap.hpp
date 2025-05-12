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
#ifndef OURS_ARCH_X86_BOOTSTRAP_HPP
#define OURS_ARCH_X86_BOOTSTRAP_HPP 1

#define X86_BOOTSTRAP_OFFSET_PGD    0
#define X86_BOOTSTRAP_OFFSET_GDTR   6
#define X86_BOOTSTRAP_OFFSET_BOOTSTRAP_ENTRY  16 

#ifndef __ASSEMBLY__

#include <ours/types.hpp>
#include <ours/init.hpp>
#include <arch/x86/descriptor.hpp>

namespace ours {
    struct PACKED BootstrapData {
        u32 pgd;
        u16 padding0_;
        arch::DescPtr gdtr;

        // It is necessary to provide us a fake PIC solution in real mode
        // and protected.
        PhysAddr bootstrap_entry;
        CpuNum this_cpu;
    };
    static_assert(offsetof(BootstrapData, pgd) == X86_BOOTSTRAP_OFFSET_PGD);
    static_assert(offsetof(BootstrapData, gdtr) == X86_BOOTSTRAP_OFFSET_GDTR);
    static_assert(offsetof(BootstrapData, bootstrap_entry) == X86_BOOTSTRAP_OFFSET_BOOTSTRAP_ENTRY);

    CXX11_CONSTEXPR
    static auto const kMinBootstrap16BufferSize = PAGE_SIZE * 3;

    INIT_CODE
    auto set_bootstrap16_buffer(PhysAddr base) -> void;

    
    INIT_CODE [[nodiscard]]
    auto make_bootstrap_area(BootstrapData **data) -> PhysAddr;

} // namespace ours

#endif // __ASSEMBLY__
#endif // #ifndef OURS_ARCH_X86_BOOTSTRAP_HPP
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
#define X86_BOOTSTRAP_OFFSET_GDTR   10
#define X86_BOOTSTRAP_OFFSET_LONGMODE_PHYS_ENTRY  16
#define X86_BOOTSTRAP_OFFSET_LONGMODE_VIRT_ENTRY  24
#define X86_BOOTSTRAP_OFFSET_CPUNUM  32 
#define X86_BOOTSTRAP_OFFSET_PERCPU  40

#ifndef __ASSEMBLY__

#include <ours/types.hpp>
#include <ours/init.hpp>
#include <ours/cpu-cfg.hpp>
#include <arch/x86/descriptor.hpp>

namespace ours {
namespace task {
    class Thread;
}

    struct BootstrapData {
        u32 pgd;
        u32 padding0_; 

        u16 padding1_;
        u16 gdtr_limit;
        u32 gdtr_base;

        u32 long_mode_phys_entry;
        u32 lond_mode_cs;

        // High half address space.
        VirtAddr long_mode_virt_entry;
        CpuNum cpunum;
        u32 padding3_;
        struct {
            VirtAddr stack_top; 
            task::Thread *current_thread; // Idle thread.
        } pcpu[MAX_CPU - 1];
    };
    static_assert(offsetof(BootstrapData, pgd) == X86_BOOTSTRAP_OFFSET_PGD);
    static_assert(offsetof(BootstrapData, gdtr_limit) == X86_BOOTSTRAP_OFFSET_GDTR);
    static_assert(offsetof(BootstrapData, long_mode_phys_entry) == X86_BOOTSTRAP_OFFSET_LONGMODE_PHYS_ENTRY);
    static_assert(offsetof(BootstrapData, long_mode_virt_entry) == X86_BOOTSTRAP_OFFSET_LONGMODE_VIRT_ENTRY);
    static_assert(offsetof(BootstrapData, cpunum) == X86_BOOTSTRAP_OFFSET_CPUNUM);
    static_assert(offsetof(BootstrapData, pcpu) == X86_BOOTSTRAP_OFFSET_PERCPU);

    CXX11_CONSTEXPR
    static auto const kMinBootstrap16BufferSize = PAGE_SIZE * 3;

    INIT_CODE
    auto set_bootstrap16_buffer(PhysAddr base) -> void;
    
    INIT_CODE [[nodiscard]]
    auto make_bootstrap_area(BootstrapData **data) -> PhysAddr;

} // namespace ours

#endif // __ASSEMBLY__
#endif // #ifndef OURS_ARCH_X86_BOOTSTRAP_HPP
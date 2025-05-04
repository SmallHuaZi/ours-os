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
#ifndef OURS_ARCH_X86_INIT_HPP
#define OURS_ARCH_X86_INIT_HPP 1

#include <ours/cpu.hpp>

namespace ours {
    /// Called once on BSP, but it is a temporary work around. 
    auto x86_init_idt_early() -> void;
    auto x86_setup_mmu_early() -> void;
    auto x86_init_feature_early() -> void;

    /// Called once on BSP, and rest of APs just call x86_load_*dt(); 
    auto x86_setup_idt() -> void;
    auto x86_setup_gdt() -> void;

    /// Called once on each CPU
    auto x86_setup_tss_percpu() -> void;
    auto x86_setup_mmu_percpu() -> void;
    auto x86_init_feature_percpu() -> void;

    /// Called in init_arch_early()
    auto x86_setup_mmu() -> void;

    auto x86_init_percpu(CpuNum cpunum) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_X86_INIT_HPP
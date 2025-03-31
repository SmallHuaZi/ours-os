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
#ifndef OURS_KERNEL_START_HPP
#define OURS_KERNEL_START_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>
#include <ours/config.hpp>
#include <ours/init.hpp>

#include <ours/phys/handoff.hpp>

namespace ours {
    FORCE_INLINE CXX11_CONSTEXPR
    auto global_handoff() -> phys::Handoff & {
        INIT_DATA
        extern phys::Handoff *PHYS_HANDOFF;

        return *PHYS_HANDOFF;
    }

    /// Set up `|handoff|` 
    INIT_CODE
    auto setup_handoff(PhysAddr handoff) -> void;

    /// Perform every set up routine required before heap/MMU is available.
    NO_MANGLE INIT_CODE
    auto init_arch_early() -> void;

    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void;

    // Perform every set up routine required after heap/MMU is available.
    NO_MANGLE INIT_CODE
    auto init_arch() -> void;

    NO_MANGLE INIT_CODE
    auto init_platform() -> void;

    NO_MANGLE INIT_CODE
    auto start_kernel(PhysAddr handoff) -> Status;

    NO_MANGLE INIT_CODE
    auto start_nonboot_cpu(CpuNum CpuNum) -> Status;

} // namespace ours

#endif // #ifndef OURS_KERNEL_START_HPP
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
#ifndef OURS_PLATFORM_INIT_HPP
#define OURS_PLATFORM_INIT_HPP 1

#include <ours/init.hpp>
#include <gktl/init_hook.hpp>

namespace ours {
    CXX11_CONSTEXPR
    static auto const kDeltaBetweenPlatformInitLevels = 0x100;

    enum class PlatformInitLevel {
        /// Represents the entry point for platform-specific early initialization.
        /// - **Stage Responsibility**: 
        ///   Listeners registered at this level must perform *non-allocating* member initialization tasks,
        ///   focusing exclusively on foundational setup of platform-critical structures. 
        ///   Strict prohibition of dynamic memory allocation ensures deterministic execution and avoids 
        ///   dependency cycles during bootstrapping phases.
        /// - **Synchronization Note**: 
        ///   This phase executes before memory management initialization, thus requiring lock-free 
        ///   synchronization mechanisms if concurrent access is possible.
        StartUp = usize(gktl::InitLevel::PlatformEarlyInternal),

        /// Serves as the preparatory stage for Physical Memory Manager (PMM) initialization.
        /// - **Primary Objective**:
        ///   Collect platform-specific memory topology metadata required for PMM's operation, such as:
        ///   - NUMA node configurations and proximity domains
        ///   - Reserved memory regions (ACPI-defined or firmware-reported)
        ///   - Heterogeneous memory ranges (HBM, persistent memory, etc.)
        /// - **Critical Constraints**:
        ///   Must complete *before* PMM initializes its internal allocators. All gathered metadata shall be
        ///   stored in pre-allocated architecture-specific buffers to avoid premature heap usage.
        PrePmm = StartUp + kDeltaBetweenPlatformInitLevels,

        Pmm = PrePmm + kDeltaBetweenPlatformInitLevels,
    };

    FORCE_INLINE CXX11_CONSTEXPR
    static auto operator+(PlatformInitLevel level, i32 offset) -> PlatformInitLevel {
        return PlatformInitLevel(ustl::traits::UnderlyingTypeT<PlatformInitLevel>(level) + offset);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto operator-(PlatformInitLevel level, i32 offset) -> PlatformInitLevel {
        return PlatformInitLevel(ustl::traits::UnderlyingTypeT<PlatformInitLevel>(level) - offset);
    }

    INIT_CODE FORCE_INLINE CXX11_CONSTEXPR
    static auto set_platform_init_level(PlatformInitLevel level) -> void {
        gktl::set_init_level(gktl::InitLevel(level), kDeltaBetweenPlatformInitLevels);
    }

    /// The followings are private to platform code.

    INIT_CODE
    auto platform_init_pmm() -> void;

    /// The followings just be public to `start_kernel`

    NO_MANGLE INIT_CODE
    auto init_platform_early() -> void;

    NO_MANGLE INIT_CODE
    auto init_platform() -> void;

} // namespace ours

#endif // #ifndef OURS_PLATFORM_INIT_HPP
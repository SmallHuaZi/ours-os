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
#ifndef GKTL_HOOK_HPP
#define GKTL_HOOK_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/views/string_view.hpp>
#include <ustl/traits/underlying.hpp>

    //namespace init_hook::NAME { \
    // }
#define GKTL_INIT_HOOK(NAME, HOOK, LEVEL) \
        LINK_SECTION(".init.rodata.init_hook") FORCE_USED \
        static gktl::InitHook const GKTL_INIT_HOOK_##NAME = { \
            HOOK,  \
            static_cast<gktl::InitLevel>(LEVEL), \
            #NAME  \
        }; \

namespace gktl {
    CXX11_CONSTEXPR
    static auto const kDeltaBetweenLevels = 0x10000;

    /// Trace the the initialization level of system during boot.
    enum class InitLevel: u32 {
        ArchEarly = kDeltaBetweenLevels,
        PlatformEarly = ArchEarly + kDeltaBetweenLevels,
        CpuLocal = PlatformEarly + kDeltaBetweenLevels,
        Heap = CpuLocal + kDeltaBetweenLevels,
        Vmm = Heap + kDeltaBetweenLevels,
        Irq = Vmm + kDeltaBetweenLevels,
        Arch = Irq + kDeltaBetweenLevels,
        Platform = Arch + kDeltaBetweenLevels,
        MaxInitLevel = Platform + kDeltaBetweenLevels,
    };

    FORCE_INLINE CXX11_CONSTEXPR
    static auto operator+(InitLevel level, i32 offset) -> InitLevel {
        return InitLevel(ustl::traits::UnderlyingTypeT<InitLevel>(level) + offset);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto operator-(InitLevel level, i32 offset) -> InitLevel {
        return InitLevel(ustl::traits::UnderlyingTypeT<InitLevel>(level) - offset);
    }

    struct InitHook {
        typedef auto (*Callback)() -> void;

        Callback    hook_;
        InitLevel   level_;
        char const *name_;
    };

    /// Setting a level causes all hooks with levels between the previous level and the 
    /// newly set |level| (exclusive) to be executed.
    auto set_init_level(InitLevel level) -> void;

    FORCE_INLINE
    static auto get_init_level() -> InitLevel {
        extern InitLevel g_init_level;
        return g_init_level;
    }

} // namespace gktl

#endif // GKTL_HOOK_HPP
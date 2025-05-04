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
#ifndef OURS_PLATFORM_HPET_HPP
#define OURS_PLATFORM_HPET_HPP 1

#include <ours/config.hpp>
#include <ours/assert.hpp>

#include <acpi/details/hpet.hpp>

namespace ours {
    struct Hpet {
        auto enable() -> void {
            mmio->general_config.enable_cnf();
        }

        auto disable() -> void {
            mmio->general_config.disable_cnf();
        }

        auto wait_for(u32 ms) -> void {
            u64 initial_ticks = mmio->main_counter_value;
            u64 target = u64(ms) * ticks_per_ms;
            while (mmio->main_counter_value - initial_ticks <= target) {}
        }

        usize ticks_per_ms;
        acpi::HpetRegs *mmio;
    };

    FORCE_INLINE
    static auto get_hpet() -> Hpet * {
        extern Hpet *g_hpet;
        return g_hpet;
    }

    FORCE_INLINE
    static auto has_hpet() -> bool {
        return get_hpet();
    }

} // namespace ours

#endif // #ifndef OURS_PLATFORM_HPET_HPP
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
#include <ours/init.hpp>

#include <acpi/details/hpet.hpp>

namespace ours {
    struct Hpet {
        auto enable() -> void {
            mmio->general_config.enable_cnf();
        }

        auto disable() -> void {
            mmio->general_config.disable_cnf();
        }

        auto configure_irq(u8 n, HIrqNum irqnum, bool mask) -> void {
            ASSERT(n < num_channels);
            mmio->timers[n].conf_caps.mask()
                                     .configure_irq(irqnum)
                                     .mask(mask);
        }

        auto wait_for(u32 ms) -> void {
            u64 initial_ticks = mmio->main_counter_value;
            u64 target = u64(ms) * ticks_per_ms;
            while (mmio->main_counter_value - initial_ticks <= target) {}
        }

        auto enable_periodic(u8 n) -> void {
            if (mmio->timers[n].conf_caps.has_periodic()) {
                mmio->timers[n].conf_caps.enable_periodic();
            } 
        }

        usize num_channels;
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

    INIT_CODE
    auto enable_hpet() -> void;

} // namespace ours

#endif // #ifndef OURS_PLATFORM_HPET_HPP
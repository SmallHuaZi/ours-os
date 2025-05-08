#include <ours/platform/timer.hpp>
#include <ours/platform/hpet.hpp>
#include <ours/task/timer.hpp>
#include <ours/irq/mod.hpp>

#include <ours/arch/apic.hpp>
#include <ours/arch/x86/feature.hpp>

#include <gktl/init_hook.hpp>
#include <ustl/algorithms/generation.hpp>
#include <logz4/log.hpp>

namespace ours {
    enum class ClockSource {
        None,

        Tsc,  //!< Time Stamp Counter.
        Hpet, //!< High Precision Event Timer.
        Pit,  //!< Programmable Interval Timer.
    };

    static ClockSource s_wall_clock;
    static ClockSource s_caliberation_clock;
    static u32 s_apic_ticks_per_ms;
    static u32 s_apic_ticks_per_ns;

    static usize s_elapsed_time_ms;
    static usize s_elapsed_ticks;

    auto get_periodic_time() -> usize {
        return 1000;
    }

    static auto platform_handle_timer_irq(HIrqNum irqnum, void *) -> irq::IrqReturn {
        DEBUG_ASSERT(irqnum == 0);

        s_elapsed_time_ms += get_periodic_time();

        task::timer_tick(get_periodic_time());
        return irq::IrqReturn::Handled;
    }

    static auto start_calibrate() -> void {
        switch (s_caliberation_clock) {
            case ClockSource::Tsc:
                break;
            case ClockSource::Hpet:
                get_hpet()->enable();
                break;
            case ClockSource::Pit:
                break;
            default:
                panic();
        }
    }

    static auto wait_for(u32 ms) -> void {
        switch (s_caliberation_clock) {
            case ClockSource::Tsc:
                break;
            case ClockSource::Hpet:
                get_hpet()->wait_for(ms);
                break;
            case ClockSource::Pit:
                break;
            default:
                panic();
        }
    }

    static auto finish_calibrate() -> void {
        switch (s_caliberation_clock) {
            case ClockSource::Tsc:
                break;
            case ClockSource::Hpet:
                get_hpet()->disable();
                break;
            case ClockSource::Pit:
                break;
            default:
                panic();
        }
    }

    static auto calibrate_apic_clock_single(u32 duration_ms, u32 apic_advisor) -> u32 {
        CXX11_CONSTEXPR
        static auto const kApicInitCount = ustl::NumericLimits<u32>::max();

        start_calibrate();

        auto status = apic_timer_set_oneshot(kApicInitCount, apic_advisor, true);
        ASSERT(Status::Ok == status);

        wait_for(duration_ms);
        u32 const apic_ticks = kApicInitCount - apic_timer_current_count();

        finish_calibrate();
        return apic_ticks;
    }

    static auto calibrate_apic_clock() -> void {
        CXX11_CONSTEXPR
        static auto const kNumTrails = 3;

        CXX11_CONSTEXPR
        static auto const kInvalidTickNum = ustl::NumericLimits<u32>::max();

        u32 best_time[kNumTrails];
        ustl::algorithms::fill_n(best_time, kNumTrails, kInvalidTickNum);

        u32 duration_ms[kNumTrails] = {2, 4, 8};
        u32 apic_advisor = 1;

        for (auto trail = 0; trail < kNumTrails; ++trail) {
            for (auto i = 0; i < 2; ++i) {
                u32 elapsed_ticks = calibrate_apic_clock_single(duration_ms[trail], apic_advisor);
                log::trace("Calibration result for duration {} ms: {} ticks/ms.", 
                    duration_ms[trail], elapsed_ticks / duration_ms[trail]);

                if (elapsed_ticks < best_time[trail]) {
                    best_time[trail] = elapsed_ticks;
                } else if (best_time[trail] == kInvalidTickNum) {
                    i -= 1;
                    apic_advisor = u8(apic_advisor << 1);
                }
            }
        }

        s_apic_ticks_per_ms = (best_time[1] - best_time[0]) / (duration_ms[1] - duration_ms[0]);
        // Low accuration but enough to now.
        s_apic_ticks_per_ns = s_apic_ticks_per_ms / 1000000;
    }

    static auto platform_init_timer() -> void {
        // We first need to pick a clock source as caliberation clock.
        // Through it we caliberate the TSC and APIC-PM.
        bool hpet = has_hpet();
        if (hpet) {
            // We always prefer the HPET.
            s_caliberation_clock = ClockSource::Hpet;
        } else {
            s_caliberation_clock = ClockSource::Pit;
        }

        if (x86_has_feature(CpuFeatureType::TscDeadlineTimer)) {
            // If there is deadline TSC, then so any needs to caliberate the APIC clock.
            init_apic_deadline_tsc();
        } else {
            calibrate_apic_clock();
        }

        enable_hpet();
        auto status = irq::request_irq(0, platform_handle_timer_irq, irq::IrqFlags(), "Timer");
        ASSERT(Status::Ok == status);
    }
    // We need to slow a step, waiting for HPET initialized.
    GKTL_INIT_HOOK(PlatformInitTimer, platform_init_timer, gktl::InitLevel::Vmm + 3);

} // namespace ours
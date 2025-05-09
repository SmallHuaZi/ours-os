#include <ours/platform/timer.hpp>
#include <ours/platform/hpet.hpp>
#include <ours/task/timer.hpp>
#include <ours/irq/mod.hpp>

#include <ours/arch/apic.hpp>
#include <ours/arch/x86/feature.hpp>

#include <gktl/init_hook.hpp>
#include <ustl/bit.hpp>
#include <ustl/ratio.hpp>
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

    /// The ratio between the chosen reference timer's ticks and the APIC's ticks.
    /// This is set after clock selection is complete in platform_init_timer.
    static ustl::Ratio<usize> s_tick_cast_ratio;

    static u32 s_apic_ticks_per_ms;
    static u32 s_apic_ticks_per_ns;
    static u32 s_apic_divisor;

    static usize s_elapsed_time_ms;
    static usize s_elapsed_ticks;

    static bool s_has_deadline_tsc;
    static bool s_has_tsc;
    static bool s_has_invariant_tsc;

    auto platform_set_oneshot_timer(Ticks deadline) -> Status {
        if (s_has_deadline_tsc) {
            apic_timer_set_tsc_deadline(deadline);
            return Status::Ok;
        }

        auto apic_ticks = deadline;
        auto const highest_set_bit = ustl::bit_floor(apic_ticks);

        u8 extra_bits = 0;
        if (highest_set_bit > 31) {
            extra_bits = highest_set_bit - 31;
        }

        u8 divisor = s_apic_divisor << extra_bits;
        usize count;
        if (divisor >= 128) {
            divisor = 128;
            count = ustl::NumericLimits<u32>::max();
        } else if (!divisor) {
            divisor = 0;
            count = apic_ticks >> extra_bits;
            if (!count) {
                count = 1;
            }
        }

        return apic_timer_set_oneshot(count, divisor, false);
    }

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

        s_apic_divisor = apic_advisor;
    }

    static auto platform_init_timer() -> void {
        s_has_deadline_tsc = x86_has_feature(CpuFeatureType::TscDeadlineTimer);
        s_has_tsc = x86_has_feature(CpuFeatureType::Tsc);
        s_has_invariant_tsc = x86_has_feature(CpuFeatureType::InvarTsc);

        // We first need to pick a clock source as caliberation clock.
        // Through it we caliberate the TSC and APIC-PM.
        bool hpet = has_hpet();
        if (s_has_invariant_tsc) {
            s_caliberation_clock = ClockSource::Tsc;
        } else if (hpet) {
            // We always prefer the HPET.
            s_caliberation_clock = ClockSource::Hpet;
        } else {
            s_caliberation_clock = ClockSource::Pit;
        }

        if (s_has_deadline_tsc) {
            // If there is deadline TSC, then so any needs to caliberate the APIC clock.
            init_apic_deadline_tsc();
        } else {
            calibrate_apic_clock();
        }

        enable_hpet();
        auto status = irq::request_irq(0, platform_handle_timer_irq, irq::IrqFlags(), "Timer");

        switch (s_caliberation_clock) {
            // case ClockSource::Tsc:
            //     s_tick_cast_ratio.assign(s_apic_ticks_per_ms, get_hpet()->ticks_per_ms);
            //     break;
            case ClockSource::Hpet:
                s_tick_cast_ratio.assign(s_apic_ticks_per_ms, get_hpet()->ticks_per_ms);
                break;
            case ClockSource::Pit:
                break;
            default:
                unreachable();
        }
        ASSERT(Status::Ok == status);
    }
    // We need to slow a step, waiting for HPET initialized.
    GKTL_INIT_HOOK(PlatformInitTimer, platform_init_timer, gktl::InitLevel::Vmm + 3);

} // namespace ours
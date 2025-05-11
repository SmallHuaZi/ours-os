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
#ifndef OURS_TASK_TIMER_HPP
#define OURS_TASK_TIMER_HPP 1

#include <ours/cpu-local.hpp>
#include <gktl/canary.hpp>

#include <ustl/collections/intrusive/list.hpp>

namespace ours::task {
    class Timer: public ustl::collections::intrusive::ListBaseHook<> {
      public:
        using OnExpired = void (*)(Timer*, Instant now, void *args);

        FORCE_INLINE
        auto deadline() const -> Instant {
            return deadline_;
        }

      private:
        GKTL_CANARY(Timer, canary_);
        Instant deadline_;
        OnExpired on_expired_;
        void *args_;
        ustl::sync::Atomic<CpuNum> active_cpu_;
    };

    class TimerQueue {
        typedef TimerQueue  Self;
      public:
        CXX11_CONSTEXPR
        static auto kInfiniteTime = ustl::NumericLimits<TimePoint>::max();

        CXX11_CONSTEXPR
        static auto kInfiniteTicks = ustl::NumericLimits<Ticks>::max();

        auto tick(CpuNum cpu) -> void;

        auto reset_preemption_timer(MonoInstant deadline) -> void;

        FORCE_INLINE
        static auto current() -> Self * {
            return CpuLocal::access(&s_timer_queue);
        }

      private:
        auto update_deadline() -> void;
        auto update_mono_deadline(MonoInstant deadline) -> void;

        static auto convert_mono_time_to_ticks(TimePoint) -> Ticks;

        GKTL_CANARY(TimerQueue, canary_);
        ustl::collections::intrusive::List<Timer> mono_timer_list_;

        Ticks next_timer_deadline_;
        MonoInstant preemption_timer_deadline_;

        static Self s_timer_queue;
    };
    CPU_LOCAL
    inline TimerQueue TimerQueue::s_timer_queue;

    auto timer_tick(usize elapsed_time_ms) -> void;

} // namespace ours::task

#endif // #ifndef OURS_TASK_TIMER_HPP
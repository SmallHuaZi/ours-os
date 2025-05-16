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
#ifndef OURS_TASK_TIMER_QUEUE_HPP
#define OURS_TASK_TIMER_QUEUE_HPP 1

#include <ours/task/timer.hpp>
#include <ours/syscall/time.hpp>

namespace ours::task {
    class TimerQueue {
        typedef TimerQueue  Self;
      public:
        CXX11_CONSTEXPR
        static auto kInfiniteTime = TimePoint::max();

        CXX11_CONSTEXPR
        static auto kInfiniteTicks = ustl::NumericLimits<Ticks>::max();

        auto tick(CpuNum cpu) -> void;

        auto reset_preemption_timer(TimePoint deadline) -> void;

        FORCE_INLINE
        static auto get() -> Self * {
            return CpuLocal::access(&s_timer_queue);
        }

        FORCE_INLINE
        static auto get(CpuNum cpu) -> Self * {
            return CpuLocal::access(&s_timer_queue, cpu);
        }
      private:
        friend class Timer;
        auto insert_timer(Timer &timer, TimePoint earliest, TimePoint latest) -> void;

        auto update_deadline() -> void;
        auto update_mono_deadline(TimePoint deadline) -> void;

        static auto convert_mono_time_to_ticks(TimePoint) -> Ticks;

        GKTL_CANARY(TimerQueue, canary_);
        Ticks next_timer_deadline_ = kInfiniteTicks;
        TimePoint preemption_timer_deadline_ = kInfiniteTime;
        ustl::collections::intrusive::List<Timer> timer_list_;

        static Self s_timer_queue;
    };
    CPU_LOCAL
    inline TimerQueue TimerQueue::s_timer_queue;

    auto timer_tick() -> void;

} // namespace ours::task

#endif // #ifndef OURS_TASK_TIMER-QUEUE_HPP
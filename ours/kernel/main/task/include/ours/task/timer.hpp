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
#include <ours/syscall/time.hpp>
#include <ours/platform/timer.hpp>

#include <gktl/canary.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/function/bind.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::task {
    class TimeSlack {
        typedef Nanoseconds DurationUnit;
      public:
        TimeSlack() = default;

        template <typename Duration>
        TimeSlack(Duration duration)
            : slack_(duration_cast<DurationUnit>(duration))
        {}

        FORCE_INLINE
        auto duration() const -> DurationUnit {
            return slack_;
        }
      private:
        DurationUnit slack_;
    };

    class Deadline {
      public:
        Deadline() = default;

        template <typename Duration>
        Deadline(Duration duration, TimeSlack slack = {})
            : time_point_(duration_cast<TimePoint::Duration>(duration) + Nanoseconds(current_mono_time())),
              time_slack_(slack)
        {}

        FORCE_INLINE
        auto when() const -> TimePoint {
            return time_point_;
        }

        FORCE_INLINE
        auto earliest() const -> TimePoint {
            return time_point_;
        }

        FORCE_INLINE
        auto latest() const -> TimePoint {
            return time_point_cast<TimePoint::Duration>(time_point_ + time_slack_.duration());
        }
      private:
        TimePoint time_point_;
        TimeSlack time_slack_;
    };

    class Timer: public ustl::collections::intrusive::ListBaseHook<> {
        typedef Timer   Self;
      public:
        using OnExpired = ustl::function::Fn<auto (Self *) -> void>;

        template <typename F, typename... Args>
        auto activate(Deadline deadline, F &&f, Args &&...args) -> void {
            OnExpired on_expired = ustl::function::bind(ustl::forward<F>(f), ustl::forward<Args>(args)...);
            return activate(deadline, on_expired);
        }

        auto activate(Deadline deadline, OnExpired on_expired) -> void;

        auto cancel() -> void;

        FORCE_INLINE
        auto on_expired() -> void {
            DEBUG_ASSERT(on_expired_);
            on_expired_(this);
        }

        FORCE_INLINE
        auto deadline() const -> TimePoint {
            return deadline_;
        }
      private:
        GKTL_CANARY(Timer, canary_);
        TimePoint deadline_;
        OnExpired on_expired_;
        ustl::sync::Atomic<CpuNum> active_cpu_;
        ustl::sync::Atomic<bool> cancalled_;
    };

} // namespace ours::task

#endif // #ifndef OURS_TASK_TIMER_HPP
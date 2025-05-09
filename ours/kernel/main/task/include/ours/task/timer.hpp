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

namespace ours::task {
    class Timer {

    };

    class TimerQueue {
        typedef TimerQueue  Self;
      public:
        auto tick() -> void;

        FORCE_INLINE
        static auto current() -> Self * {
            return CpuLocal::access(&s_timer_queue);
        }

      private:
        static Self s_timer_queue;
    };

    auto timer_tick(usize elapsed_time_ms) -> void;

} // namespace ours::task

#endif // #ifndef OURS_TASK_TIMER_HPP
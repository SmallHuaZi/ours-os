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
#ifndef OURS_SCHED_RUNQUEUE_HPP
#define OURS_SCHED_RUNQUEUE_HPP 1

#include <ours/sched/types.hpp>

namespace ours::sched {
    template <typename T>
    class RunQueue {
        typedef RunQueue    Self;
      public:
        RunQueue() = default;
        ~RunQueue() = default;

        auto enqueue(T &obj) -> void {
            // TODO(SmallHuaZi) Implement this.
        }

        auto dequeue(T &obj) -> void {
            // TODO(SmallHuaZi) Implement this.
        }
      private:
        friend class Scheduler;
        SchedWeight weight_;
    };

} // namespace ours::sched

#endif // #ifndef OURS_SCHED_RUNQUEUE_HPP
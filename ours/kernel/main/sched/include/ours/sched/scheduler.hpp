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
#ifndef OURS_SCHED_SCHEDULER_HPP
#define OURS_SCHED_SCHEDULER_HPP 1

#include <ours/sched/cfg.hpp>
#include <ours/sched/types.hpp>

#include <ustl/views/span.hpp>
#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    class IScheduler {
        typedef IScheduler  Self;
      public:
        virtual auto enqueue(SchedObject &obj) -> void = 0;

        virtual auto dequeue(SchedObject &obj) -> void = 0;

        virtual auto yield() -> void = 0;

        virtual auto yield(SchedObject &obj) -> void = 0;

        virtual auto pick_next() -> SchedObject * = 0;

        virtual auto set_next(SchedObject &obj) -> void = 0;

        virtual auto on_tick() -> void = 0;

        OURS_CFG_SCHED(MIGRATE, auto migrate(SchedObject &obj)->void);
    };

    /// The main per cpu scheduler.
    class MainScheduler {
      public:
        static auto preempt() -> void;

        static auto reschedule(SchedObject &) -> void;

        /// The routine called in timer interrupt routine.
        static auto tick() -> void;
      private:
        usize nr_runnable_;
        ustl::views::Span<IScheduler *> schedulers_;
    };

} // namespace ours::sched

#define OURS_SCHEDULER_API \
    virtual auto pick_next() -> SchedObject *override;\
    virtual auto enqueue(SchedObject &obj) -> void override;\
    virtual auto dequeue(SchedObject &obj) -> void override;\
    virtual auto on_tick() -> void override;

#endif // #ifndef OURS_SCHED_SCHEDULER_HPP

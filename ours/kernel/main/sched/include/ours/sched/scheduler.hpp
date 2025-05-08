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

#include <ours/sched/types.hpp>
#include <ours/mutex.hpp>
#include <ours/cpu-local.hpp>

#include <ours/task/thread.hpp>

#include <ustl/views/span.hpp>
#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    struct SchedCommonData {
        /// Calculate the scheduling period
        auto scheduling_period() const -> SchedTime {
            return scheduling_period_grans * minimal_granularity;
        }

        auto minimal_scheduling_period() const -> SchedTime {
            return minimal_period_grans  * minimal_granularity;
        }

        task::Thread *curr_thread_;
        task::Thread *prev_thread_;

        // Virtual time.
        SchedTime timeline;
        SchedTime vruntime;

        // Physical time.
        SchedTime previous_started_time;
        SchedTime current_started_time;
        SchedTime current_preemption_time;

        /// Scheduling period in which every runnable task executes once in units of
        /// minimal granularity.
        usize scheduling_period_grans; // Unit: min_granularity

        /// Scheduling period in which every runnable task executes once in units of
        /// minimal granularity.
        usize minimal_period_grans;

        /// The smallest timeslice a thread is allocated in a single round.
        SchedTime minimal_granularity;

        SchedWeight total_weight;
    };

    class IScheduler {
        typedef IScheduler  Self;
      public:
        virtual auto enqueue(SchedObject &obj) -> void = 0;

        virtual auto dequeue(SchedObject &obj) -> void = 0;

        virtual auto yield() -> void = 0;

        virtual auto yield(SchedObject &obj) -> void = 0;

        virtual auto evaluate_next() -> SchedObject * = 0;

        virtual auto set_next(SchedObject &obj) -> void = 0;

        virtual auto on_tick() -> void = 0;
      protected:
        friend class MainScheduler;

        FORCE_INLINE
        auto current_time() const -> SchedTime {
            return common_data_->timeline;
        }

        usize num_runnable_;
        SchedCommonData *common_data_;
    };

    /// The main per cpu scheduler.
    class MainScheduler {
        typedef MainScheduler   Self;
      public:
        CXX11_CONSTEXPR
        static auto const kDefaultMinSchedGranularity = Milliseconds(1);

        /// Each process then runs for a “timeslice” proportional to its weight divided by the total weight of 
        /// all runnable threads. To calculate the actual timeslice, CFS sets a target for its approximation of 
        /// the “infinitely small” scheduling duration in perfect multitasking. This target is called the targeted 
        /// latency. Smaller targets yield better interactivity and a closer approximation to perfect multitasking, 
        /// at the expense of higher switching costs and thus worse overall throughput.
        CXX11_CONSTEXPR
        static auto const kDefaultMinTargetedLatency = Milliseconds(8);

        auto init() -> void;

        auto preempt() -> void;

        auto reschedule() -> void;

        auto activate(task::Thread &) -> void;

        auto deactivate(task::Thread &) -> void;

        /// The routine called in timer interrupt routine.
        auto tick() -> void;

        FORCE_INLINE
        auto num_runnable() const -> usize {
            return num_runnable_;
        }

        FORCE_INLINE
        static auto current() -> Self * {
            return CpuLocal::access<Self>(&s_main_scheduler);
        }

        auto init_thread(task::Thread &, BaseProfile const &) -> void;
      private:
        auto enqueue_thread(task::Thread &, SchedTime now) -> void;

        auto dequeue_thread(task::Thread &) -> void;

        auto update_timeline(SchedTime now) -> void;

        auto reschedule_common(task::Thread *, SchedTime now) -> void;

        auto evaluate_next_thread(task::Thread *, SchedTime now) -> task::Thread *;

        FORCE_INLINE
        auto set_prev_thread(task::Thread *thread) -> void {
            common_data_.prev_thread_ = thread;
        }

        enum SchedulerType {
            kEevdf,
        };

        Mutex mutex_;
        usize num_runnable_;
        usize num_switches_;
        SchedCommonData common_data_;
        ustl::views::Span<IScheduler *> schedulers_;

        static Self s_main_scheduler;
    };

} // namespace ours::sched

#define OURS_SCHEDULER_API \
    virtual auto evaluate_next() -> SchedObject *override;\
    virtual auto enqueue(SchedObject &obj) -> void override;\
    virtual auto dequeue(SchedObject &obj) -> void override;\
    virtual auto on_tick() -> void override;

#endif // #ifndef OURS_SCHED_SCHEDULER_HPP

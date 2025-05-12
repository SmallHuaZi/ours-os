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
#include <ours/platform/timer.hpp>

#include <ustl/views/span.hpp>
#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    struct SchedCommonData {
        CXX11_CONSTEXPR
        static auto const kDefaultMinSchedGranularity = Milliseconds(1);

        /// Each process then runs for a “timeslice” proportional to its weight divided by the total weight of 
        /// all runnable threads. To calculate the actual timeslice, CFS sets a target for its approximation of 
        /// the “infinitely small” scheduling duration in perfect multitasking. This target is called the targeted 
        /// latency. Smaller targets yield better interactivity and a closer approximation to perfect multitasking, 
        /// at the expense of higher switching costs and thus worse overall throughput.
        CXX11_CONSTEXPR
        static auto const kDefaultMinTargetedLatency = Milliseconds(8);

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
        SchedTime last_updated_time_;
        SchedDuration total_expected_runtime_;

        /// Scheduling period in which every runnable task executes once in units of
        /// minimal granularity.
        usize scheduling_period_grans = kDefaultMinTargetedLatency / kDefaultMinSchedGranularity;

        /// Scheduling period in which every runnable task executes once in units of
        /// minimal granularity.
        usize minimal_period_grans = kDefaultMinTargetedLatency / kDefaultMinSchedGranularity;

        /// The smallest timeslice a thread is allocated in a single round.
        SchedDuration minimal_granularity = kDefaultMinSchedGranularity;

        SchedWeight weight_sum_;
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
        virtual auto put_prev(SchedObject &obj) -> void = 0;

        virtual auto on_tick() -> void = 0;
      protected:
        friend class MainScheduler;

        FORCE_INLINE
        auto current_time() const -> SchedTime {
            return common_data_->timeline;
        }

        GKTL_CANARY(Scheduler, canary_);
        usize num_runnable_;
        SchedCommonData *common_data_;
    };

    /// The main per cpu scheduler.
    class MainScheduler {
        typedef MainScheduler   Self;
      public:
        FORCE_INLINE
        static auto get() -> Self * {
            return CpuLocal::access<Self>(&s_main_scheduler);
        }

        FORCE_INLINE
        static auto get(CpuNum target_cpu) -> Self * {
            return CpuLocal::access<Self>(&s_main_scheduler, target_cpu);
        }

        FORCE_INLINE
        static auto set_current_thread(task::Thread *current) -> void {
            get()->common_data_.curr_thread_ = current;
        }

        FORCE_INLINE
        static auto current_time() -> SchedTime {
            return SchedTime(current_mono_time());
        }

        FORCE_INLINE
        static auto peek_active_mask() -> CpuMask {
            return s_active_schedulers.load();
        }

        FORCE_INLINE
        static auto tick(SchedTime now) -> void {
            get()->on_tick();
        }

        static auto block(task::Thread &) -> void;

        static auto unblock(task::Thread &) -> void;

        static auto preempt() -> void;

        static auto reschedule(task::Thread &) -> void;

        /// The routine called in timer interrupt routine.
        auto on_tick() -> void;

        auto init() -> void;

        FORCE_INLINE
        auto num_runnable() const -> usize {
            return num_runnable_;
        }

        FORCE_INLINE
        auto is_active() const -> bool {
            return s_active_schedulers.load().test(this_cpu_);
        }

        auto init_thread(task::Thread &, BaseProfile const &) -> void;
      private:
        static auto assign_target_cpu(task::Thread &) -> CpuNum;

        auto enqueue_thread(task::Thread &, SchedTime now) -> void;

        auto dequeue_thread(task::Thread &) -> void;

        auto update_timeline(SchedTime now) -> void;

        auto reschedule_common(task::Thread *, SchedTime now) -> void;

        auto evaluate_next_thread(task::Thread *, SchedTime now) -> task::Thread *;

        auto put_prev_thread(task::Thread *thread) -> void;
        auto set_next_thread(task::Thread *thread) -> void;

        enum SchedulerType {
            kEevdf,
        };

        GKTL_CANARY(MainScheduler, canary_);
        Mutex mutex_;
        CpuNum this_cpu_;
        usize num_runnable_;
        usize num_switches_;
        SchedCommonData common_data_;
        ustl::views::Span<IScheduler *> schedulers_;

        ai_percpu static Self s_main_scheduler;
        static inline ustl::sync::Atomic<CpuMask> s_active_schedulers;
        static inline ustl::sync::Atomic<CpuMask> s_idle_schedulers;
    };

    CPU_LOCAL
    inline MainScheduler MainScheduler::s_main_scheduler;

} // namespace ours::sched

#define OURS_SCHEDULER_API \
    virtual auto evaluate_next() -> SchedObject *override;\
    virtual auto enqueue(SchedObject &obj) -> void override;\
    virtual auto dequeue(SchedObject &obj) -> void override;\
    virtual auto on_tick() -> void override;\
    virtual auto set_next(SchedObject &obj) -> void override;\
    virtual auto put_prev(SchedObject &obj) -> void override;\
    virtual auto yield() -> void override;\
    virtual auto yield(SchedObject &obj) -> void override;

#endif // #ifndef OURS_SCHED_SCHEDULER_HPP

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

#include <ours/task/types.hpp>
#include <ours/mutex.hpp>
#include <ours/cpu-local.hpp>

#include <ours/task/thread.hpp>
#include <ours/platform/timer.hpp>

#include <ustl/views/span.hpp>
#include <ustl/chrono/duration.hpp>

namespace ours::task {
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

        Thread *prev_thread_;

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
        virtual auto enqueue_thread(Thread *thread) -> void = 0;
        virtual auto dequeue_thread(Thread *thread) -> void = 0;

        virtual auto yield() -> void = 0;

        virtual auto yield_to(Thread *thread) -> void = 0;

        virtual auto pick_next_thread(Thread *curr) -> Thread * = 0;

        virtual auto set_next_thread(Thread *curr) -> void = 0;
        virtual auto put_prev_thread(Thread *curr) -> void = 0;

        virtual auto on_tick() -> void = 0;
      protected:
        friend class MainScheduler;

        FORCE_INLINE
        auto current_time() const -> SchedTime {
            return common_data_->timeline;
        }

        GKTL_CANARY(IScheduler, canary_);
        usize num_runnable_;
        SchedCommonData *common_data_;
    };

    /// The main per cpu scheduler.
    class MainScheduler {
        typedef MainScheduler   Self;
      public:
        class Current;

        FORCE_INLINE
        static auto current_time() -> SchedTime {
            return SchedTime(current_mono_time());
        }

        FORCE_INLINE
        static auto peek_active_mask() -> CpuMask {
            return s_active_schedulers.load();
        }

        auto deactivate_thread(Thread *) -> void;

        /// Deactivate a thread. This function will do the following thins:
        ///     1) Select a suitable scheduler for the given thread.
        ///     2) Add the thread onto the scheduler's running queue.
        ///     3) Drop the thread's lock. 
        ///     4) Reschedule if needed/permitted.
        ///
        /// Assumption:
        ///     1) The thread has been locked.
        auto activate_thread(Thread *) -> void;

        /// The routine called in timer interrupt routine.
        auto on_tick() -> void;

        auto init() -> void;

        auto init_thread(Thread &, BaseProfile const &) -> void;

        auto reschedule(Thread *, SchedTime now) -> void;

        auto run() -> void;

        FORCE_INLINE
        auto num_runnable() const -> usize {
            return num_runnable_;
        }

        FORCE_INLINE
        auto is_active() const -> bool {
            return s_active_schedulers.load().test(this_cpu_);
        }
      private:
        static auto assign_target_cpu(Thread *) -> CpuNum;

        static auto reset_preemption(CpuNum cpu, TimePoint now, TimePoint deadline) -> void;

        auto enqueue_thread(Thread *, SchedTime now) -> void;

        auto dequeue_thread(Thread *, SchedTime now) -> void;

        auto update_timeline(SchedTime now) -> void;

        auto switch_context(Thread *curr, Thread *next) -> void;

        // Attempts to steal work from other busy CPUs and move it to the local run
        // queues. Returns a pointer to the stolen thread that is now associated with
        // the local Scheduler instance, or nullptr is no work was stolen.
        auto steal_work() -> Thread *;

        /// Pick next most eligible thread and return it. Under multi-processor environment,
        /// it will attempts to steal a task on remote CPUs.
        auto pick_next_thread(Thread *, SchedTime now) -> Thread *;

        auto put_prev_and_set_next_thread(Thread *prev, Thread *next) -> void;

        GKTL_CANARY(MainScheduler, canary_);
        Mutex mutex_;
        CpuNum this_cpu_;
        usize num_runnable_;
        usize num_switches_;
        SchedCommonData common_data_;
        ustl::views::Span<IScheduler *> schedulers_;

        Thread *idler_;

        static inline ustl::sync::Atomic<CpuMask> s_active_schedulers;
        static inline ustl::sync::Atomic<CpuMask> s_idle_schedulers;
    };

    class MainScheduler::Current {
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
        static auto tick(SchedTime now) -> void {
            get()->on_tick();
        }

        static auto preempt() -> void;

        FORCE_INLINE
        static auto schedule() -> void {
            auto thread = Thread::Current::get();
            get()->reschedule(thread, SchedTime(current_mono_time()));
        }
      private:
        CPU_LOCAL
        static inline MainScheduler s_main_scheduler;
    };

} // namespace ours::task

#define OURS_SCHEDULER_API \
    virtual auto enqueue_thread(Thread *thread) -> void override;\
    virtual auto dequeue_thread(Thread *thread) -> void override;\
    virtual auto yield() -> void override;\
    virtual auto yield_to(Thread *thread) -> void override;\
    virtual auto pick_next_thread(Thread *curr) -> Thread * override;\
    virtual auto set_next_thread(Thread *curr) -> void override;\
    virtual auto put_prev_thread(Thread *curr) -> void override;\
    virtual auto on_tick() -> void override;

#endif // #ifndef OURS_SCHED_SCHEDULER_HPP

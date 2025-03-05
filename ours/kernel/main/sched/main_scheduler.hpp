#ifndef OURS_SCHED_MAIN_SCHEDULER_HPP
#define OURS_SCHED_MAIN_SCHEDULER_HPP

#include <ours/cpu.hpp>
#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_states.hpp>

#include <ustl/sync/atomic.hpp>
#include <ustl/collections/array.hpp>

#ifndef NR_SCHEDULER
#   define NR_SCHEDULER 4
#endif

namespace ours::sched {
    struct SchedCommonData
    {
        usize  runnable_task_count_;
        usize  uninterruptible_task_count_;

        usize  switches_count_;
        usize  iowaitings_count_;

        typedef ustl::chrono::Nanoseconds   TimeUnit;
        TimeUnit minimum_timeslice_;
        TimeUnit timeslice_granularity_;
        TimeUnit total_virtual_time_;

        SchedWeight total_weight_;
    };

    /// Per cpu holds one instance.
    class MainScheduler
    {
    public:
        auto reschedule() -> void;

    private:
        CpuId this_cpu_;

        typedef ustl::collections::Array<Scheduler *, NR_SCHEDULER> SchedulerSet;
        SchedulerSet schedulers_;

        SchedCommonData common_data_;

        static ustl::sync::AtomicU32 ACTIVE_SCDEDULERS_;
        static ustl::sync::AtomicU32 IDLE_SCDEDULERS_;
    };
}

#endif // #ifndef OURS_SCHED_MAIN_SCHEDULER_HPP
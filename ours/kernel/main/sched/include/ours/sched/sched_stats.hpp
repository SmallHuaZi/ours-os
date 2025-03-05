#ifndef OURS_SCHED_STATS_HPP
#define OURS_SCHED_STATS_HPP 1

#include <ours/types.hpp>
#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    struct SchedStats
    {
        typedef ustl::chrono::Nanoseconds   TimeUnit;
        usize  runnable_task_count_;
        usize  iowaiting_task_count_;
        usize  uninterruptible_task_count_;

        usize  switches_count_;

        TimeUnit total_virtual_time_;
    }; // class SchedStats

} // namespace ours::sched

#endif // #ifndef OURS_SCHED_STATS_HPP
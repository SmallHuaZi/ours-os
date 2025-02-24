#ifndef OURS_SCHED_SCHED_STATES_HPP
#define OURS_SCHED_SCHED_STATES_HPP 1

#include <ours/types.hpp>

#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    struct SchedTime {
        typedef ustl::chrono::Nanoseconds   TimeUnit; 
        TimeUnit vs_time_;
        TimeUnit ve_time_;

        TimeUnit runtime_;
        TimeUnit time_slice_;
        TimeUnit expected_time_slice_;
    };

    struct SchedWeight {

    };

    struct SchedStates {
        SchedTime   time_;
        SchedWeight weight_;
    };

} // namespace ours::sched

#endif // #ifndef OURS_TASK_SCHED_STATES_H
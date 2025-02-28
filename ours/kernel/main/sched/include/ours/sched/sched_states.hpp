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

#ifndef OURS_SCHED_SCHED_STATES_HPP
#define OURS_SCHED_SCHED_STATES_HPP 1

#include <ours/types.hpp>

#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    struct SchedTime {
        typedef ustl::chrono::Nanoseconds   TimeUnit; 
        TimeUnit vs_time_; // Virtual start time.
        TimeUnit ve_time_; // Virtual end time.

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

#endif // #ifndef OURS_SCHED_SCHED_STATES_HPP
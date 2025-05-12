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
#ifndef OURS_SYSCALL_TIME_HPP
#define OURS_SYSCALL_TIME_HPP 1

#include <ustl/chrono/duration.hpp>

namespace ours {
    using ustl::chrono::Femtoseconds;
    using ustl::chrono::Picoseconds;
    using ustl::chrono::Nanoseconds;
    using ustl::chrono::Milliseconds;
    using ustl::chrono::Microseconds;
    using ustl::chrono::Seconds;
    using ustl::chrono::Minutes;
    using ustl::chrono::Years;

    using TimePoint = ustl::chrono::TimePoint<void, Nanoseconds>;

    using ustl::chrono::duration_cast;
    using ustl::chrono::time_point_cast;

} // namespace ours

#endif // #ifndef OURS_SYSCALL_TIME_HPP
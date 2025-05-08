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
#ifndef OURS_SCHED_TYPES_HPP
#define OURS_SCHED_TYPES_HPP 1

#include <ours/types.hpp>

#include <ustl/chrono/duration.hpp>
#include <cnl/fraction.hpp>

namespace ours::sched {
    using ustl::chrono::Nanoseconds;
    using ustl::chrono::Microseconds;
    using ustl::chrono::Milliseconds;

    typedef ustl::chrono::Nanoseconds   SchedTime;

    /// High weight accompanied by low virtual time flow rate.
    ///
    /// The unit convertion between real time and virtual as following:
    ///     V(t) = t / TotalWeight;
    ///     t = V(t) * TotalWeight;
    ///
    /// For a task, 
    /// define S(i) as the service time obtained by the task i
    /// define Ve(i) = the start time point of S(i) and the eligible time in EEVDF
    /// define Vd(i) = the end time point of S(i)
    ///     S(i) = (Vd(i) - Ve(i)) * W(i);
    ///     Vd(i) = Ve(i) + Si(i) / W(i)
    typedef cnl::Fraction<usize, cnl::Power<16>>    SchedWeight;

    class SchedObject;
    class IScheduler;
    class MainScheduler;

} // namespace ours::sched

#endif // #ifndef OURS_SCHED_TYPES_HPP
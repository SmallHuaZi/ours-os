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
#ifndef OURS_PLATFORM_TIMER_HPP
#define OURS_PLATFORM_TIMER_HPP 1

#include <ours/types.hpp>

#include <ustl/chrono/duration.hpp>

namespace ours {
    auto current_mono_ticks() -> MonoTicks;

    auto current_mono_time() -> usize;

    /// Unit: ms
    auto get_periodic_time() -> usize;

} // namespace ours

#endif // #ifndef OURS_PLATFORM_TIMER_HPP
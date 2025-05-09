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
#ifndef OURS_SCHED_MOD_HPP
#define OURS_SCHED_MOD_HPP 1

#include <ours/task/thread.hpp>

namespace ours::sched {
    auto activate_thread(task::Thread *) -> void;

    auto deactivate_thread(task::Thread *) -> void;

    auto reschedule() -> void;

} // namespace ours::sched

#endif // #ifndef OURS_SCHED_MOD_HPP
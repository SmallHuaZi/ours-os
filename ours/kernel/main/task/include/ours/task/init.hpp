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
#ifndef OURS_TASK_INIT_HPP
#define OURS_TASK_INIT_HPP 1

namespace ours::task {
    auto init_task_early() -> void;

    auto init_task() -> void;

} // namespace ours::task

#endif // #ifndef OURS_TASK_INIT_HPP
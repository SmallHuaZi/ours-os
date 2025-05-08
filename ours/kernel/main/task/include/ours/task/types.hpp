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

#ifndef OURS_TASK_TYPES_HPP
#define OURS_TASK_TYPES_HPP 1

// #include <ustl/function/fn.hpp>

namespace ours::task {
    using ThreadStartEntry = auto (*)() -> void;// ustl::function::Fn<auto () -> void>;

    class Thread;
    

} // namespace ours::task

#endif // #ifndef OURS_TASK_TYPES_HPP
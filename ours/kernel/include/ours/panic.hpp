// ours OURS_PANIC_HPP
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

#ifndef OURS_PANIC_HPP
#define OURS_PANIC_HPP 1

#include <logz4/log.hpp>

namespace ours {
    auto panic() -> void;

    template <typename... Args> 
    auto panic(char const *fmt, Args&&... args) -> void
    {
        log::error(fmt, args...);
        panic();
    }

} // namespace ours

#endif // #ifndef OURS_PANIC_HPP
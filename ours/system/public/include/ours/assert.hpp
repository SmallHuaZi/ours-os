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
#ifndef OURS_ASSERT_HPP
#define OURS_ASSERT_HPP 1

#include <ours/panic.hpp>

#define ASSERT(condition, ...)  \
    if (!(condition)) {\
        ours::panic(__VA_ARGS__);\
    }

#define DEBUG_ASSERT(condition, ...) \
    if constexpr (OURS_DEBUG) {\
        if (!(condition)) {\
            ours::panic(__VA_ARGS__);\
        }\
    }

#endif // #ifndef OURS_ASSERT_HPP
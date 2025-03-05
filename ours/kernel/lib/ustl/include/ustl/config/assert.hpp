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

#ifndef USTL_CONFIG_ASSERT_HPP
#define USTL_CONFIG_ASSERT_HPP 1

#define USTL_RUNTIME_ERROR_MESSAGE(msg)       "[ustl rumtime error]: " ## msg
#define USTL_COMPILE_TIME_ERROR_MESSAGE(msg)  "[ustl compile error]: " ## msg

#ifdef USTL_DEBUG
    #define USTL_ASSERT(expr, ...)
    #define USTL_INVARIANT_ASSERT(expr, ...)  \
        USTL_ASSERT(expression, __VA_ARGS__)
#else
    #define USTL_ASSERT(expr, ...)
    #define USTL_INVARIANT_ASSERT(expr, ...)
#endif

#define USTL_STATIC_ASSERT(expr, ...)   \
    static_assert(expr, USTL_FORMAT_CTI(__VA_ARGS__))

#undef USTL_FORMAT_RTI
#undef USTL_FORMAT_CTI

#endif // #ifndef USTL_CONFIG_ASSERT_HPP
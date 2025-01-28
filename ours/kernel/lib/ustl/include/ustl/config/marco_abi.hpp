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

#ifndef USTL_CONFIG_MARCO_ABI_HPP
#define USTL_CONFIG_MARCO_ABI_HPP 1

//! Example:
//! #define USTL_MPL_VAINVOKER_PREFIX  MY_INVOKER_PREFIX
//! #define MY_INVOKER_PREFIX_0()      [] () { std::cout << "Zero parameter"; };
//! #define MY_INVOKER_PREFIX_1(...)   [] () { std::cout << "One parameter " << __VA_ARGS__; };
//! ...
//! 
//! Do following operations in main function:
//! USTL_MPL_VAMATCH();             // Output: "Zero parameter".
//! USTL_MPL_VAMATCH("hello");      // Output: "One parameter hello".

#define USTL_MPL_INTERNAL_VAINVOKER_0(...) \
    USTL_MPL_CONCAT(USTL_MPL_VAINVOKER_PREFIX, _##TAG(__VA_ARGS__))

#define USTL_MPL_CONCAT_EXPAND(a, b) a ## b
#define USTL_MPL_CONCAT(a, b)  \
    USTL_MPL_CONCAT_EXPAND(a, b)

#define USTL_MPL_VACOUNT_ARGS_AUX(                  \
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,         \
    _10, _11, _12, _n, X...) _n

#define USTL_MPL_VACOUNT_ARGS(...)                  \
    USTL_MPL_VACOUNT_ARGS_AUX(,## __VA_ARGS__, 12, 11,    \
    10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#ifndef USTL_MPL_GET_BRANCH_TAG 
    #define USTL_MPL_GET_BRANCH_TAG USTL_MPL_VACOUNT_ARGS
#endif

#define USTL_MPL_VAMATCH_IMPL_EXPAND_PREFIX(PREFIX, ARGS)  \
    USTL_MPL_CONCAT(PREFIX, ARGS)

#define USTL_MPL_VAMATCH_IMPL_EXPAND_TAG(TAG, ...)  \
    USTL_MPL_VAMATCH_IMPL_EXPAND_PREFIX(USTL_MPL_VAINVOKER_PREFIX, _##TAG(__VA_ARGS__))

#define USTL_MPL_VAMATCH_IMPL_EXPAND(TAG, ...)  \
    USTL_MPL_VAMATCH_IMPL_EXPAND_TAG(TAG, ## __VA_ARGS__)

#define USTL_MPL_VAMATCH(...)  \
    USTL_MPL_VAMATCH_IMPL_EXPAND(USTL_MPL_GET_BRANCH_TAG(__VA_ARGS__), ## __VA_ARGS__)

#endif // #ifndef USTL_CONFIG_MARCO_ABI_HPP
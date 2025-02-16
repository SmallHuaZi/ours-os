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

#ifndef OURS_CONFIG_IF_CFG_HPP
#define OURS_CONFIG_IF_CFG_HPP 1

#define OURS_INTERNAL_EMPTY_CONTENT
#define OURS_INTERNAL_ARG_PLACEHOLDER_0 0, OURS_INTERNAL_EMPTY_CONTENT,
#define OURS_INTERNAL_CONTENT_WRAPPER(...)  __VA_ARGS__
#define OURS_INTERNAL_TAKE_CONTENT(__ignored0, content, ...) content

#define OURS_INTERNAL_IS_DEFINED(x, ...)                \
    OURS_INTERNAL_IS_DEFINED_1(x, __VA_ARGS__)

#define OURS_INTERNAL_IS_DEFINED_1(x, ...)			    \
    OURS_INTERNAL_IS_DEFINED_2(x, __VA_ARGS__)

#define OURS_INTERNAL_IS_DEFINED_2(x, ...)		    \
    OURS_INTERNAL_IS_DEFINED_3(OURS_INTERNAL_ARG_PLACEHOLDER_##x, __VA_ARGS__)

#define OURS_INTERNAL_IS_DEFINED_3(x, ...)	        \
    OURS_INTERNAL_TAKE_CONTENT(x, OURS_INTERNAL_CONTENT_WRAPPER(__VA_ARGS__))

/// Helper macros to use OURS_CONFIG_ options in C/CPP expressions. Note that
/// these only work with boolean and tristate options.
///
/// Getting something that works in C and CPP for an arg that may or may
/// not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
/// we match on the placeholder define, insert the "0," for arg1 and generate
/// the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
/// When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
/// the last step cherry picks the 2nd arg, we get a zero.
///
/// Use as:
///     1). OURS_IF_CFG(NUMA, 
///             CXX11_CONSTEXPR 
///             usize const MAX_NODES = 32
///         );
///     2). enum Types: usize {
///             OURS_IF_CFG(ZONE_DMA, Dma,)
///             OURS_IF_CFG(ZONE_DMA32, Dma32,)
///             Normal,
///             OURS_IF_CFG(ZONE_MOVABLE, Movable,)
///             MaxCount,
///         };

#define OURS_IF_CFG(option, ...)    \
    OURS_INTERNAL_IS_DEFINED(OURS_CONFIG_##option, __VA_ARGS__) 

#define OURS_IF_NOT_CFG(option, ...)

#endif // #ifndef OURS_CONFIG_IF_CFG_HPP

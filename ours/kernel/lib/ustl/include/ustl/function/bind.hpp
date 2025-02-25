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

#ifndef USTL_FUNCTION_BIND_HPP
#define USTL_FUNCTION_BIND_HPP 1

#include <functional>

namespace ustl::function {
    using std::placeholders::__ph;
    using std::bind;

    constexpr inline __ph<1> _1;
    constexpr inline __ph<2> _2;
    constexpr inline __ph<3> _3;
    constexpr inline __ph<4> _4;
    constexpr inline __ph<5> _5;
    constexpr inline __ph<6> _6;
    constexpr inline __ph<7> _7;
    constexpr inline __ph<8> _8;
    constexpr inline __ph<9> _9;
    constexpr inline __ph<10> _10;

} // namespace ustl::function

#endif // #ifndef USTL_FUNCTION_BIND_HPP
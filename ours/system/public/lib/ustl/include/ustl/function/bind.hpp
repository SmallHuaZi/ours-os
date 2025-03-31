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
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;
    using std::placeholders::_5;
    using std::placeholders::_6;
    using std::placeholders::_7;
    using std::placeholders::_8;
    using std::placeholders::_9;
    using std::placeholders::_10;

    using std::bind;
    using std::bind_front;
    using std::bind_back;

} // namespace ustl::function

#endif // #ifndef USTL_FUNCTION_BIND_HPP
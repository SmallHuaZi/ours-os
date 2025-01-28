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

#ifndef USTL_UTIL_NONCOPYABLE_HPP
#define USTL_UTIL_NONCOPYABLE_HPP 1

#define USTL_NOMOVEABLE(TYPE) TYPE(TYPE &&) = delete;
#define USTL_NOCOPYABLE(TYPE) TYPE(TYPE const &) = delete;

#define USTL_NO_MOVEABLE_AND_COPYABLE(TYPE) \
    USTL_NOMOVEABLE(TYPE) \
    USTL_NOCOPYABLE(TYPE)

namespace ustl {
    struct NonConstructible {
        NonConstructible() = delete;
    };

    struct NonMovable {
        NonMovable(NonMovable &&) = delete;
    };
     
    struct NonCopyable {
        NonCopyable(NonCopyable const &) = delete;
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_NONCOPYABLE_HPP
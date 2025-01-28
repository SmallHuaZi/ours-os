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

#ifndef GKTL_STATIC_OBJECTS_HPP
#define GKTL_STATIC_OBJECTS_HPP 1

#include <ours/config.hpp>
#include <ours/early.hpp>

namespace gktl {
    typedef void (*Ctor)();
    NO_MANGLE Ctor const CTORS_START[];
    NO_MANGLE Ctor const CTORS_END[];

    /// `init_static_objects` is called only once.
    EARLY_CODE
    auto init_static_objects() -> void;

} // namespace gktl

#endif // #ifndef GKTL_STATIC_OBJECTS_HPP
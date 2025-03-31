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
#ifndef HEAP_LIB_HPP
#define HEAP_LIB_HPP 1

#include <ours/config.hpp>

namespace heap {
    auto malloc() -> void;

    auto free() -> void;

} // namespace heap

#endif // #ifndef HEAP_LIB_HPP
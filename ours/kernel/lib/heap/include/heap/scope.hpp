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

#ifndef HEAP_NEW_HPP
#define HEAP_NEW_HPP 1

#include <ours/types.hpp>
#include <ours/config/lang_items.hpp>

namespace heap {
    template <typename T, typename... Options> 
    struct ObjectCache;
} // namespace heap

auto operator new(ours::usize size, ours::usize align) -> void *;

template <typename T, typename... Options> 
auto operator new(ours::usize size, ours::usize align, heap::ObjectCache<T, Options...> *) -> void *;

#endif // #ifndef HEAP_NEW_HPP
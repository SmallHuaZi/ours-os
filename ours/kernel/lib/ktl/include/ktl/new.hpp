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
#ifndef KTL_NEW_HPP
#define KTL_NEW_HPP 1

#include <ktl/kmalloc.hpp>

auto operator new(usize size, ktl::Gaf gaf) -> void *;

auto operator new[](usize size, ktl::Gaf gaf) -> void *;

auto operator new(usize size, ktl::Gaf gaf, ktl::NodeId nid) -> void *;

auto operator new[](usize size, ktl::Gaf gaf, ktl::NodeId nid) -> void *;

auto operator new(usize size, ours::mem::ObjectCache &, ktl::Gaf gaf) -> void *;

auto operator new[](usize size, ours::mem::ObjectCache &, ktl::Gaf gaf) -> void *;

auto operator new(usize size, ours::mem::ObjectCache &, ktl::Gaf gaf, ktl::NodeId nid) -> void *;

auto operator new[](usize size, ours::mem::ObjectCache &, ktl::Gaf gaf, ktl::NodeId nid) -> void *;

auto operator delete(void *ptr) -> void;

auto operator delete[](void *ptr) -> void;

#endif // #ifndef KTL_NEW_HPP
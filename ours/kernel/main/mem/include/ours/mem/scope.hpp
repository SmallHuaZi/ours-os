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
#ifndef OURS_MEM_SCOPE_HPP
#define OURS_MEM_SCOPE_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marker.hpp>

#include <ours/mem/gaf.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/pmm.hpp>

#include <arch/cache.hpp>

namespace ours {
namespace mem {
    auto kmalloc(usize size, Gaf gaf, NodeId nid) -> void *;
    auto kmalloc(usize size, Gaf gaf) -> void *;
    auto kfree(void *ptr) -> void;
}
    CXX11_CONSTEXPR
    static auto const kPageAlign = AlignVal(PAGE_SIZE);

    CXX11_CONSTEXPR
    static auto const kMaxPageAlign = AlignVal(MAX_PAGE_SIZE);

    CXX11_CONSTEXPR
    static auto const kCacheAlign = AlignVal(arch::kCacheSize);
} // namespace ours

FORCE_INLINE
auto operator new(usize size, ours::AlignVal align, ours::mem::Gaf gaf, ours::NodeId nid) -> void * {
    return ours::mem::kmalloc(size, gaf, nid);
}

FORCE_INLINE
auto operator new(usize size, ours::mem::Gaf gaf, ours::NodeId nid) -> void * {
    return ours::mem::kmalloc(size, gaf, nid);
}

FORCE_INLINE
auto operator new[](usize size, ours::mem::Gaf gaf, ours::NodeId nid) -> void * {
    return ours::mem::kmalloc(size, gaf, nid);
}

#endif // #ifndef OURS_MEM_SCOPE_HPP
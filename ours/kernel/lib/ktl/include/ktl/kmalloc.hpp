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
#ifndef KTL_KMALLOC_HPP
#define KTL_KMALLOC_HPP 1

#include <ktl/types.hpp>

namespace ktl {
    auto init_kmalloc() -> void;

    auto kmalloc(usize size, Gaf gaf) -> void *;

    auto kmalloc(usize size, Gaf gaf, NodeId nid) -> void *;

    auto kfree(void *object) -> void;

} // namespace ktl

#endif // #ifndef KTL_KMALLOC_HPP
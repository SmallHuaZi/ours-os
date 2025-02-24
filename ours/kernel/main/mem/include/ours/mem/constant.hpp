// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///

#ifndef OURS_MEM_CONSTANT_HPP
#define OURS_MEM_CONSTANT_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marco_abi.hpp>

#include <ours/mem/cfg.hpp>

#include <ustl/bit.hpp>

namespace ours::mem {
    CXX11_CONSTEXPR
    static usize const FRAME_SHIFT = OURS_CONFIG_FRAME_SHIFT;

    CXX11_CONSTEXPR
    static usize const FRAME_SIZE = BIT(FRAME_SHIFT);

    CXX11_CONSTEXPR
    static usize const FRAME_MASK = FRAME_SIZE - 1;

    CXX11_CONSTEXPR
    static usize const SECTION_SHIFT = OURS_CONFIG_SECTION_SHIFT;

    CXX11_CONSTEXPR
    static usize const SECTION_SIZE = BIT(SECTION_SHIFT);

    CXX11_CONSTEXPR
    static usize const SECTION_MASK = SECTION_SIZE - 1;

    CXX11_CONSTEXPR
    static usize const PAGE_SHIFT = FRAME_SHIFT;

    CXX11_CONSTEXPR
    static usize const PAGE_SIZE = BIT(PAGE_SHIFT);

    CXX11_CONSTEXPR
    static usize const MAX_PAGE_SHIFT = FRAME_SHIFT;

    CXX11_CONSTEXPR
    static usize const MAX_PAGE_SIZE = BIT(MAX_PAGE_SHIFT);

    CXX11_CONSTEXPR 
    static usize const MAX_NODES = OURS_CONFIG_MAX_NODES;

    CXX11_CONSTEXPR 
    static usize const MAX_NODES_BITS = ustl::bit_width(MAX_NODES);

    CXX11_CONSTEXPR
    static usize const NR_ZONES_PER_NODE = OURS_CONFIG_NR_ZONES_PER_NODE;

    CXX11_CONSTEXPR
    static usize const NR_ZONES_PER_NODE_BITS = ustl::bit_width(NR_ZONES_PER_NODE);

    CXX11_CONSTEXPR
    static usize const MAX_ZONES = MAX_NODES * NR_ZONES_PER_NODE;

    CXX11_CONSTEXPR
    static usize const MAX_ZONES_BITS = ustl::bit_width(MAX_ZONES);

    CXX11_CONSTEXPR
    static usize const NR_FRAME_ORDERS  = OURS_CONFIG_NR_FRAME_ORDERS;

    CXX11_CONSTEXPR
    static usize const MAX_FRAME_ORDER  = NR_FRAME_ORDERS - 1;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_CONSTANT_HPP
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

#ifndef OURS_MEM_CONSTANT_HPP
#define OURS_MEM_CONSTANT_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marco_abi.hpp>

#include <ours/mem/cfg.hpp>

namespace ours::mem {
    CXX11_CONSTEXPR
    static usize const FRAME_SHIFT = OURS_CONFIG_FRAME_SHIFT;

    CXX11_CONSTEXPR
    static usize const FRAME_SIZE = BIT(FRAME_SHIFT);

    CXX11_CONSTEXPR
    static usize const PAGE_SHIFT = FRAME_SHIFT;

    CXX11_CONSTEXPR
    static usize const PAGE_SIZE = BIT(PAGE_SHIFT);

    CXX11_CONSTEXPR 
    static usize const MAX_NR_NODES = OURS_CONFIG_MAX_NR_NODES;

    CXX11_CONSTEXPR
    static usize const MAX_NR_ZPIS = OURS_CONFIG_MAX_NR_ZPIS;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_CONSTANT_HPP
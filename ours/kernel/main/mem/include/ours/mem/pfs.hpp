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

#ifndef OURS_MEM_PF_FLAGS_HPP
#define OURS_MEM_PF_FLAGS_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/bit.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
namespace pfns {
    /// `PfStates` is a shorthand of physical frame states.
    enum PfStatesBits {
        ActiveBit = NR_ZONES_PER_NODE_BITS + MAX_NODES_BITS,
        PinnedBit,
        DirtyBit,
        UpToDateBit,
        ForeignBit,
        BeingWaitingBit,
        FolioBit,
        ReclaimableBit,
        MaxNumPfsBits,
    };
    CXX11_CONSTEXPR
    static usize const PFSTATES_BITS = ustl::bit_width<usize>(MaxNumPfsBits);

    enum PfStates: usize {
        Active   = BIT(ActiveBit),
        Pinned   = BIT(PinnedBit),
        Dirty    = BIT(DirtyBit),
        UpToDate = BIT(UpToDateBit),
        Foreign  = BIT(ForeignBit),
    };
    USTL_ENABLE_ENUM_BITS(PfStates);

    enum PfRole: usize {
        Io = BIT(MaxNumPfsBits),
        Lru,
        Mmu,
        Pmm,
        Heap,
        MaxNumPfRole,
    };
    CXX11_CONSTEXPR
    static usize const PFROLE_BITS = ustl::bit_width<usize>(MaxNumPfRole);

    FORCE_INLINE CXX11_CONSTEXPR
    static auto get_role(usize flags) -> PfRole
    {
        CXX11_CONSTEXPR
        auto const rolbootmemsk = BIT_RANGE(MaxNumPfsBits, PFROLE_BITS);
        return PfRole(flags & rolbootmemsk);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto set_role(usize &flags, PfRole role) -> void
    {
        CXX11_CONSTEXPR
        auto const rolbootmemsk = BIT_RANGE(MaxNumPfsBits, PFROLE_BITS);
        flags &= ~rolbootmemsk; 
        flags |= role;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto get_zone_type(usize flags) -> ZoneType
    {
        CXX11_CONSTEXPR
        auto const ztmask = BIT_RANGE(MAX_NODES_BITS, MAX_NODES_BITS + NR_ZONES_PER_NODE_BITS);
        return ZoneType((flags & ztmask) >> MAX_NODES_BITS);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto set_zone_type(usize &flags, ZoneType type) -> void 
    {
        CXX11_CONSTEXPR
        auto const ztmask = BIT_RANGE(MAX_NODES_BITS, MAX_NODES_BITS + NR_ZONES_PER_NODE_BITS);
        flags &= ~ztmask;
        flags |= type << MAX_NODES_BITS;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto get_node_id(usize flags) -> NodeId 
    {
        CXX11_CONSTEXPR
        auto const nidmask = BIT_RANGE(0, MAX_NODES_BITS);
        return NodeId(flags & nidmask);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto set_node_id(usize &flags, NodeId nid) -> void 
    {
        CXX11_CONSTEXPR
        auto const nidmask = BIT_RANGE(0, MAX_NODES_BITS);
        flags &= ~nidmask;
        flags |= nid;
    }

} // namespace ours::mem::pfns
    typedef pfns::PfStates      PfStates;
    typedef pfns::PfRole        PfRole;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PF_FLAGS_HPP
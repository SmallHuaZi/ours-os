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

#ifndef OURS_MEM_FLAGS_HPP
#define OURS_MEM_FLAGS_HPP

#include <ours/mem/constant.hpp>
#include <ours/mem/memory_priority.hpp>

#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    /// `Gaf` is a shordhand of getting available frame flags.
    ///
    /// The lowest 6 bits act as explantation of 
    /// the way of a frame to be used, so it must be equal to 'PfFlags'. 
    ///
    /// The other bits will be used to provide more details about a allocation.
    ///
    enum class Gaf: u64 {
        // __* is pre-defined member, please do not use.

        // First two bits are used to code information about priority of zone
        __FirstBitOfFramePriority,
        __LastBitOfFramePriority,
        // Then five bits are used to code infomation about node id.
        __FirstBitOfNodeId,
        __LastBitOfNodeId = 5 + __LastBitOfFramePriority,
        __FirstBitOfControlFlags,
        __RequiredBit = __FirstBitOfControlFlags,
        __OnlyThisNodeBit,
        __RetryUntilBit,
        __ContiguousBit,
        __UniqueFrameBit,
        __ReclaimBit,
        __LastBitOfControlFlags = __ReclaimBit,

        /// 
        __NrBitsOfFramePriority = __LastBitOfFramePriority - __FirstBitOfFramePriority + 1,
        __NrBitsOfNodeId = __LastBitOfNodeId - __FirstBitOfNodeId + 1,

        /// Interface fields.
        Required     = BIT(__RequiredBit),
        OnlyThisNode = BIT(__OnlyThisNodeBit),
        RetryUntil   = BIT(__RetryUntilBit),
        Contiguous   = BIT(__ContiguousBit),
        UniqueFrame  = BIT(__UniqueFrameBit),

        /// If the remaining and available memory is not enough for the request of user, 
        /// this bit-flag allows the PMM to reclaim memory from lru list. 
        Reclaim     = BIT(__ReclaimBit),
    };
    USTL_ENABLE_ENUM_BITS(Gaf);

    typedef ustl::traits::UnderlyingTypeT<Gaf>  GafVal;

    static_assert(usize(Gaf::__LastBitOfControlFlags) <= 32, "");

    FORCE_INLINE CXX11_CONSTEXPR
    static auto gaf_get_frame_priority(Gaf gaf) -> MemoryPriority 
    {
        CXX11_CONSTEXPR
        auto const shift = GafVal(Gaf::__FirstBitOfFramePriority);

        CXX11_CONSTEXPR
        auto const mask = (1 << GafVal(Gaf::__NrBitsOfFramePriority)) - 1;
        return MemoryPriority((static_cast<GafVal>(gaf) << shift) & mask);
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto gaf_get_node_id(Gaf gaf) -> NodeId 
    {
        CXX11_CONSTEXPR
        auto const shift = GafVal(Gaf::__FirstBitOfNodeId);

        CXX11_CONSTEXPR
        auto const mask = (1 << GafVal(Gaf::__NrBitsOfNodeId)) - 1;
        return NodeId((static_cast<GafVal>(gaf) << shift) & mask);
    }

    CXX11_CONSTEXPR
    static auto const GAF_BOOT = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const GAF_USER = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const GAF_KERNEL = Gaf::OnlyThisNode | Gaf::Reclaim;

} // namespace ours::mem

#endif // OURS_MEM_FLAGS_HPP
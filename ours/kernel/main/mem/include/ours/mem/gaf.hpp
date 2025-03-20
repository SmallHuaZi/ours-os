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

#include <ours/mem/types.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
namespace gafns {
    enum GafBits: usize {
        DmaBit,
        Dma32Bit,
        RequiredBit,
        OnlyThisNodeBit,
        NeverFailBit,
        ZeroBit,
        ReclaimBit,
        DirectlyReclaimBit,
    };

    /// `Gaf` is a shordhand of getting available frame.
    ///
    enum Gaf: usize {
        Dma             = BIT(DmaBit),
        Dma32           = BIT(Dma32Bit),
        Required        = BIT(RequiredBit),
        OnlyThisNode    = BIT(OnlyThisNodeBit),

        /// `PMM` will retry infinitely until it get an available frame.
        NeverFail       = BIT(NeverFailBit),
        Zero            = BIT(ZeroBit),

        /// In process of an allocation, it indicates `PMM` under memory pressure
        /// that it could enable the back-side thread to reclaim frames and sleep
        /// applicant's thread to avoid wasting CPU time.
        Reclaim         = BIT(ReclaimBit),

        /// Allow `PMM` to directly reclaim frames under certain memory pressure,
        /// meaning that the process of reclaim is linear so the applicant does not
        /// wait through sleeping.
        DirectlyReclaim = BIT(DirectlyReclaimBit),
    };
    USTL_ENABLE_ENUM_BITS(Gaf);

    FORCE_INLINE CXX11_CONSTEXPR
    static auto zone_type(Gaf gaf) -> ZoneType
    {
        // ZONE_SHIFT 
        CXX11_CONSTEXPR
        auto const GAF_ZONE_SHIFT = MAX_ZONES_BITS - 1;

        CXX11_CONSTEXPR
        auto const GAF_ZONE_MASK = Gaf::Dma | Gaf::Dma32;

        CXX11_CONSTEXPR
        Gaf const ZONE_TABLE {
            BIT(Gaf::Dma * GAF_ZONE_SHIFT)   |
            BIT(Gaf::Dma32 * GAF_ZONE_SHIFT)
        };

        auto const bits = gaf & GAF_ZONE_MASK;
        auto zone = ZONE_TABLE >> (bits * GAF_ZONE_SHIFT);
        return ZoneType((1 << bits) - 1);
    }

} // namespace ours::mem::gafns
    // Export Gaf for user
    using gafns::Gaf;

    CXX11_CONSTEXPR
    static auto const GAF_BOOT = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const GAF_USER = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const GAF_KERNEL = Gaf::OnlyThisNode | Gaf::Reclaim;

} // namespace ours::mem

#endif // OURS_MEM_FLAGS_HPP
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
#ifndef OURS_MEM_GAF_HPP
#define OURS_MEM_GAF_HPP

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
    USTL_ENABLE_ENUM_BITMASK(Gaf);

    FORCE_INLINE CXX11_CONSTEXPR
    static auto gaf_zone_type(Gaf gaf) -> ZoneType {
        // ZONE_SHIFT 
        CXX11_CONSTEXPR
        auto const kGafZoneShift = MAX_ZONES_BITS - 1;

        CXX11_CONSTEXPR
        auto const kGafZoneMask = Gaf::Dma | Gaf::Dma32;

        CXX11_CONSTEXPR
        Gaf const kZoneTable {
            BIT(Gaf::Dma * kGafZoneShift)   |
            BIT(Gaf::Dma32 * kGafZoneShift)
        };

        auto const bits = gaf & kGafZoneMask;
        auto zone = kZoneTable >> (bits * kGafZoneShift);
        return ZoneType((1 << bits) - 1);
    }

} // namespace ours::mem::gafns
    using gafns::Gaf;
    using gafns::gaf_zone_type;

    CXX11_CONSTEXPR
    static auto const kGafBoot = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const kGafUser= Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const kGafKernel = Gaf::OnlyThisNode | Gaf::Reclaim;

} // namespace ours::mem

#endif // OURS_MEM_GAF_HPP

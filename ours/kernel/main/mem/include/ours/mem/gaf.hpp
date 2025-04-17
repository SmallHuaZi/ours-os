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
    /// `Gaf` is a shordhand of getting available frame.
    ///
    enum class Gaf: usize {
        __DmaBit,
        __Dma32Bit,
        __RequiredBit,
        __OnlyThisNodeBit,
        __NeverFailBit,
        __ZeroBit,
        __ReclaimBit,
        __DirectlyReclaimBit,

        Dma             = BIT(__DmaBit),
        Dma32           = BIT(__Dma32Bit),
        Required        = BIT(__RequiredBit),
        OnlyThisNode    = BIT(__OnlyThisNodeBit),

        /// `PMM` will retry infinitely until it get an available frame.
        NeverFail       = BIT(__NeverFailBit),
        Zero            = BIT(__ZeroBit),

        /// In process of an allocation, it indicates `PMM` under memory pressure
        /// that it could enable the back-side thread to reclaim frames and sleep
        /// applicant's thread to avoid wasting CPU time.
        Reclaim         = BIT(__ReclaimBit),

        /// Allow `PMM` to directly reclaim frames under certain memory pressure,
        /// meaning that the process of reclaim is linear so the applicant does not
        /// wait through sleeping.
        DirectlyReclaim = BIT(__DirectlyReclaimBit),
    };
    USTL_ENABLE_ENUM_BITMASK(Gaf);

    typedef ustl::traits::UnderlyingTypeT<Gaf>  GafVal;

    FORCE_INLINE CXX11_CONSTEXPR
    static auto gaf_zone_type(Gaf gaf) -> ZoneType {
        // ZONE_SHIFT 
        CXX11_CONSTEXPR
        auto const kGafZoneShift = MAX_ZONES_BITS - 1;

        CXX11_CONSTEXPR
        auto const kGafZoneMask = Gaf::Dma | Gaf::Dma32;

        CXX11_CONSTEXPR
        GafVal const kZoneTable {
            BIT(GafVal(Gaf::Dma) * kGafZoneShift)   |
            BIT(GafVal(Gaf::Dma32) * kGafZoneShift)
        };

        auto const bits = GafVal(gaf & kGafZoneMask);
        auto zone = kZoneTable >> (GafVal(bits) * kGafZoneShift);
        return ZoneType((1 << bits) - 1);
    }

    CXX11_CONSTEXPR
    static auto const kGafBoot = Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const kGafUser= Gaf::OnlyThisNode;

    CXX11_CONSTEXPR
    static auto const kGafKernel = Gaf::OnlyThisNode | Gaf::Reclaim | Gaf::Dma | Gaf::Dma32;

} // namespace ours::mem

#endif // OURS_MEM_GAF_HPP

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
    enum class GafBit: usize {
        ZoneDmaBit,
        ZoneDma32Bit,
        ZoneNormalBit,
        RequiredBit,
        OnlyThisNodeBit,
        NeverFailBit,
        ZeroBit,
        ReclaimBit,
        DirectlyReclaimBit,
        FolioBit,
    };

    /// `Gaf` is a shordhand of getting available frame.
    ///
    enum class Gaf: usize {
        ZoneDma         = BIT(usize(GafBit::ZoneDmaBit)),
        ZoneDma32       = BIT(usize(GafBit::ZoneDma32Bit)),
        ZoneNormal      = BIT(usize(GafBit::ZoneNormalBit)),
        Required        = BIT(usize(GafBit::RequiredBit)),
        OnlyThisNode    = BIT(usize(GafBit::OnlyThisNodeBit)),

        /// `PMM` will retry infinitely until it get an available frame.
        NeverFail       = BIT(usize(GafBit::NeverFailBit)),
        Zero            = BIT(usize(GafBit::ZeroBit)),

        /// In process of an allocation, it indicates `PMM` under memory pressure
        /// that it could enable the back-side thread to reclaim frames and sleep
        /// applicant's thread to avoid wasting CPU time.
        Reclaim         = BIT(usize(GafBit::ReclaimBit)),

        /// Allow `PMM` to directly reclaim frames under certain memory pressure,
        /// meaning that the process of reclaim is linear so the applicant does not
        /// wait through sleeping.
        DirectlyReclaim = BIT(usize(GafBit::DirectlyReclaimBit)),

        Folio           = BIT(usize(GafBit::FolioBit)),
    };
    USTL_ENABLE_ENUM_BITMASK(Gaf);

    typedef ustl::traits::UnderlyingTypeT<Gaf>  GafVal;

    FORCE_INLINE CXX11_CONSTEXPR
    static auto gaf_zone_type(Gaf gaf) -> ZoneType {
        // ZONE_SHIFT 
        CXX11_CONSTEXPR
        auto const kGafZoneMask = Gaf::ZoneDma | Gaf::ZoneDma32 | Gaf::ZoneNormal;

        CXX11_CONSTEXPR
        GafVal const kZoneTable {
            GafVal(ZoneType::Dma) << GafVal(Gaf::ZoneDma) * ZONES_SHIFT      |
            GafVal(ZoneType::Dma32) << GafVal(Gaf::ZoneDma32) * ZONES_SHIFT  |
            GafVal(ZoneType::Normal) << GafVal(Gaf::ZoneNormal) * ZONES_SHIFT
        };

        // Extract the bits that indicates the type of zone.
        auto const bits = GafVal(gaf & kGafZoneMask);
        auto zone = (kZoneTable >> (GafVal(bits) * ZONES_SHIFT)) & ((1 << ZONES_SHIFT) - 1);
        return ZoneType(zone);
    }
    static_assert(gaf_zone_type(Gaf::ZoneDma) == ZoneType::Dma);
    static_assert(gaf_zone_type(Gaf::ZoneDma32) == ZoneType::Dma32);
    static_assert(gaf_zone_type(Gaf::ZoneNormal) == ZoneType::Normal);

    CXX11_CONSTEXPR
    static auto const kGafBoot = Gaf::OnlyThisNode | Gaf::ZoneNormal | Gaf::Zero;

    CXX11_CONSTEXPR
    static auto const kGafKernel = Gaf::OnlyThisNode | Gaf::Reclaim | Gaf::ZoneNormal;

    CXX11_CONSTEXPR
    static auto const kGafUser= Gaf::OnlyThisNode;

} // namespace ours::mem

#endif // OURS_MEM_GAF_HPP

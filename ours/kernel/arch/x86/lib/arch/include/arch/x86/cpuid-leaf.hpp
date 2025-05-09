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
#ifndef ARCH_PROCESSOR_CPUID_LEAF_HPP
#define ARCH_PROCESSOR_CPUID_LEAF_HPP

#include <arch/types.hpp>

namespace arch {
    enum class CpuIdLeaf: u32 {
        Vendor,
        Features,
        CacheV1,
        CacheV2 = 4,
        Monitor,
        ThermalAndPower,
        ExtendedFeature,
        PerformanceMonitoring = 0xa,
        Topology = 0xb,
        Xsave = 0xd,
        Pt = 0x14,
        Tsc = 0x15,

        HypVendor = 0x40000000,
        HypFeatures = 0x40000001,

        IntelExtended = 0x80000000,
        IntelFeatures,
        IntelBrand,
        Amd80000007EBX = 0x80000007,
        IntelAddrWidth = 0x80000008,
        AmdTopology = 0x8000001e,
    };

    enum class CpuIdSubLeaf: u32 {};
}

#endif // #ifndef ARCH_PROCESSOR_CPUID_LEAF_HPP
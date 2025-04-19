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
#ifndef OURS_ARCH_MEM_CFG_HPP
#define OURS_ARCH_MEM_CFG_HPP 1

#include <ours/mem/init.hpp>
#include <ustl/util/type-list.hpp>

#define ARCH_MAX_DMA_PFN        (MB(16) >> PAGE_SHIFT)
#define ARCH_MAX_DMA32_PFN      (MB(512) >> PAGE_SHIFT)
#define ARHC_MAX_PFN_LIMIT      (GB(64) >> PAGE_SHIFT)

namespace ours {
    using ArchPmmZoneConfig = ustl::TypeList<
         mem::MaxZonePfn<mem::ZoneType::Dma, ARCH_MAX_DMA_PFN, OURS_CONFIG_ZONE_DMA>,
         mem::MaxZonePfn<mem::ZoneType::Dma32, ARCH_MAX_DMA32_PFN, OURS_CONFIG_ZONE_DMA32>,
         mem::MaxZonePfn<mem::ZoneType::Normal, ARHC_MAX_PFN_LIMIT>
     >;
} // namespace ours

#endif // #ifndef OURS_ARCH_MEM_CFG_HPP
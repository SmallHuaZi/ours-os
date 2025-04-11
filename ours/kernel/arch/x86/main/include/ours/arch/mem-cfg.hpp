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

namespace ours {
    using ArchPmmZoneConfig = ustl::TypeList<
        mem::MaxZonePfn<mem::Dma, (MB(16) >> PAGE_SHIFT)>,
        mem::MaxZonePfn<mem::Dma32, (GB(32) >> PAGE_SHIFT)>,
        mem::MaxZonePfn<mem::Normal, (GB(64) >> PAGE_SHIFT)>
    >;
} // namespace ours

#endif // #ifndef OURS_ARCH_MEM_CFG_HPP
// ours/mem OURS/MEM_MEMORY_PRIORITY_HPP
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

#ifndef OURS_MEM_MEMORY_PRIORITY_HPP
#define OURS_MEM_MEMORY_PRIORITY_HPP 1

#include <ours/mem/types.hpp>

#include <ustl/views/span.hpp>

namespace ours::mem {
    enum class MemoryPriority {
        Low,
        Normal,
        High,
        Critical,
        Max,
    };

    CXX11_CONSTEXPR 
    static usize const MAX_NR_ZONES_PER_NODE = usize(MemoryPriority::Max);

    CXX11_CONSTEXPR 
    static usize const MAX_NR_ZONES = MAX_NR_NODES * MAX_NR_ZONES_PER_NODE;

    struct ZonePriorityInfo
    {
        auto get_priority_if_contains(PhysAddr base, usize len) const -> MemoryPriority 
        {
            auto start_pfn = phys_to_pfn(base);
            auto end_pfn = phys_to_pfn(base + len - 1);

            if (start <= start_pfn && end_pfn < end) {
                return priority;
            }

            return MemoryPriority::Max;
        }

        Pfn start;
        Pfn end;
        MemoryPriority priority;
    };

    struct ZonePartitioner
    {
        auto partition(ustl::views::Span<ZonePriorityInfo> zones, PhysAddr base, usize len) const -> MemoryPriority
        {
            for (const auto& zone : zones) {
                auto priority = zone.get_priority_if_contains(base, len);
                if (priority != MemoryPriority::Max) {
                    return priority;
                }
            }
            return MemoryPriority::Normal; // Default priority if no match is found
        }
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_PRIORITY_HPP
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

#ifndef OURS_MEM_INIT_HPP
#define OURS_MEM_INIT_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/memory_priority.hpp>

#include <ours/early.hpp>
#include <ours/status.hpp>

#include <ustl/views/span.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    /// Perform the initialization of physical memory manager.
    /// \p `zpis` gives out of the priority topology among non-overlapping memory 
    /// regions. Such as the following example showed:
    ///     [0x0, 0xFFFF]           MemoryPriority::Low,
    ///     [0x10000, 0x1FFFF]      MemoryPriority::Normal,
    ///     [0x20000, 0x3FFFF]      MemoryPriority::High,
    ///     [0x40000, 0x5FFFF]      MemoryPriority::Critical,
    ///     [0x60000, 0x7FFFF]      MemoryPriority::Normal,
    /// But it is not required, 
    ///
    EARLY_CODE
    auto init_pmm(ustl::views::Span<ZonePriorityInfo> = {}) -> Status;

    /// Perform the initialization of virtual memory manager.
    EARLY_CODE
    auto init_vmm() -> Status;

    /// Perform the initialization of heap memory allocation.
    EARLY_CODE
    auto init_heap() -> Status;

    enum class RemTag {
        All,
        Mem,
        Task,
        Sched,
    };
    EARLY_CODE
    auto reclaim_early_memory(RemTag = RemTag::All) -> Status;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_INIT_HPP
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

#include <ours/init.hpp>
#include <ours/status.hpp>
#include <ours/phys/handoff.hpp>

#include <ustl/views/span.hpp>

#include <ours/phys/arch-bootmem.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    INIT_CODE
    auto handoff_early_pmm(phys::MemoryHandoff &) -> Status;

    /// Perform the initialization of physical memory manager. Call this function after
    /// invoking the `setup_nodes`.
    ///
    /// |zpis| gives out of the priority topology among non-overlapping memory 
    /// regions. Such as the following example showed:
    ///     [0x0, 0xFFFF]           MemoryPriority::Low,
    ///     [0x10000, 0x1FFFF]      MemoryPriority::Normal,
    ///     [0x20000, 0x3FFFF]      MemoryPriority::High,
    ///     [0x40000, 0x5FFFF]      MemoryPriority::Critical,
    ///     [0x60000, 0x7FFFF]      MemoryPriority::Normal,
    /// But it is not required, 
    ///
    INIT_CODE
    auto init_pmm(ustl::views::Span<Pfn> pfn = {}) -> void;

    /// Perform the initialization of virtual memory manager.
    INIT_CODE
    auto init_vmm() -> void;

    enum class RemTag {
        All,
        Mem,
        Task,
        Sched,
    };
    INIT_CODE
    auto reclaim_init_area(RemTag = RemTag::All) -> Status;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_INIT_HPP
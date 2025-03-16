// ours/phys OURS/PHYS_ARCH_HANDOFF_HPP
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

#ifndef OURS_PHYS_ARCH_HANDOFF_HPP
#define OURS_PHYS_ARCH_HANDOFF_HPP 1

#include <ours/types.hpp>
#include <ustl/views/span.hpp>

namespace ours::phys {
    struct X86CpuTopologyNode {
        enum Type {
            Cluster,
            // Identical with Socket
            Package,
            Die,
            DieGrp,
            Tile,
            Module,
            Core,
            LogicalProcessor,
        };

        u32 flags;
        u32 apic_id;
        ustl::views::Span<X86CpuTopologyNode> s;
    };

    struct CpuTopologyNode {};

    struct ArchHandOff {};

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_ARCH_HANDOFF_HPP
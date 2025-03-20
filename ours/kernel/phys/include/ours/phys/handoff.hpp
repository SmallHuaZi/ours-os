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
#ifndef OURS_PHYS_HANDOFF_HPP
#define OURS_PHYS_HANDOFF_HPP 1

#include <ours/assert.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/node_mask.hpp>
#include <ours/phys/arch_handoff.hpp>

#include <bootmem/bootmem.hpp>
#include <ustl/views/span.hpp>

namespace ours::phys {
    struct MemoryHandoff {
        PhysAddr bootmem_phys_base;
        PhysAddr bootmem_phys_size;
        PhysAddr bootmem;
        PhysAddr kernel_load_addr;
        u32 stack_size;
    };

    ///
    struct Handoff {
        CXX11_CONSTEXPR
        static u32 const MAGIC = 0xA1B2C3D4;

        FORCE_INLINE
        auto verify() const -> void {
            DEBUG_ASSERT(magic == MAGIC, "Invalid handoff.");
        }

        u32 const magic = MAGIC;
        PhysAddr efi_system_table;
        PhysAddr acpi_rsdp;
        MemoryHandoff mem;
        ustl::views::Span<CpuTopologyNode> cpu_topology;

        // Optional and only to provide unusal quirks from a part of architectures, so put it here.
        ArchHandOff arch;
    };
    static_assert(sizeof(Handoff) <= PAGE_SIZE, "the size of Handoff must be lesser than 4096 for the zero page");

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_HANDOFF_HPP
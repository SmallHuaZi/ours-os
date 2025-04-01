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

#include <ours/cpu_mask.hpp>
#include <ours/assert.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/node_mask.hpp>
#include <ours/phys/arch-handoff.hpp>
#include <ours/phys/arch-bootmem.hpp>

#include <ustl/views/span.hpp>
#include <ustl/function/fn.hpp>

namespace ours::phys {
    struct EarlyMem {
        template <typename T>
        using Fn = ustl::function::Fn<T>;

        // auto protect(base, size) -> void 
        Fn<auto (PhysAddr, PhysAddr) -> void> protect;

        // auto (size, alignment, start, end, nid) -> PhysAddr
        Fn<auto (usize, usize, PhysAddr, PhysAddr, mem::NodeId) -> PhysAddr> allocate;

        // auto deallocate(base, size) -> void
        Fn<auto (PhysAddr, PhysAddr) -> void> deallocate;

        // auto deallocate(BootMem::IterationContext &) -> void
        Fn<auto (BootMem::IterationContext &) -> ustl::Option<bootmem::Region>> iterate;

        PhysAddr start_address;
        PhysAddr end_address;
    };

    struct MemoryHandoff {
        EarlyMem bootmem;
        PhysAddr kernel_load_addr;
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
        u32 nr_cpus;
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
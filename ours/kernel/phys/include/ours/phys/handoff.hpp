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
#include <bootmem/bootmem.hpp>

#include <ustl/views/span.hpp>

namespace ours::phys {
    struct MemoryHandoff
    {
        PhysAddr const bootmem_phys_base;
        usize const bootmem_phys_size;

        /// During the transition from `phys` to `main`, employing polymorphism is highly inadvisable. 
        /// This would forcibly require the `main` module's page tables to maintain stub entries for 
        /// the physical memory regions occupied by `phys`. Although perilous, this approach remains 
        /// feasible.
        ///
        /// It serve as the heart of `EarlyMem`
        ai_virt mutable bootmem::IBootMem *bootmem;
    };

    /// 
    struct Handoff
    {
        CXX11_CONSTEXPR
        static u32 const MAGIC = 0xA1B2C3D4;

        auto verify() const -> void
        {  DEBUG_ASSERT(magic == MAGIC, "Invalid handoff."); }

        u32 const magic = MAGIC;
        MemoryHandoff mem;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_HANDOFF_HPP
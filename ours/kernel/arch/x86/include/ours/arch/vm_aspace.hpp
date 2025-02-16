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

#ifndef OURS_ARCH_VM_ASPACE_HPP
#define OURS_ARCH_VM_ASPACE_HPP 1

#include <ours/arch/x86/page_table.hpp>

#include <ours/cpu_mask.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    class ArchVmAspace
    {
        typedef ArchVmAspace  Self;
    public:
        ArchVmAspace(VirtAddr base, usize size, VmasFlags flags);

        static auto context_switch(Self *from, Self *to) -> void;

        auto init() -> Status;

        auto map(VirtAddr, usize n, MmuFlags flags) -> Status;

        auto map_to(PhysAddr, VirtAddr, usize n, MmuFlags flags) -> Status;

        auto unmap(VirtAddr, usize n, MmuFlags flags) -> Status;

        auto query(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status;

        auto mark_accessed(VirtAddr, usize) -> Status
        {  return Status::Unsupported;  }

        auto harvest_accessed(VirtAddr va, usize n, HarvestAction action) -> Status;

        auto active_cpus() const -> CpuMask;
    
    private:
        GKTL_CANARY(ArchVmAspace, canary_);

        /// CPUs that are currently executing in this virtual address space.
        CpuMask active_cpus_;

        VmasFlags flags_;
        gktl::Range<VirtAddr> range_;
        union {
            alignas(X86PageTableMmu) char _0[sizeof(X86PageTableMmu)];
            alignas(X86PageTableEpt) char _1[sizeof(X86PageTableEpt)];
        } page_table_storaged_;
        IX86PageTable *page_table_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_VM_ASPACE_HPP 
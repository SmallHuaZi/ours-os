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
#ifndef OURS_MEM_ARCH_VM_ASPACE_HPP
#define OURS_MEM_ARCH_VM_ASPACE_HPP 1

#include <ours/mem/arch_vm_aspace.hpp>
#include <ours/arch/x86/page_table.hpp>
#include <ours/cpu-mask.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    class ArchVmAspace {
        typedef ArchVmAspace  Self;
    public:
        ArchVmAspace(VirtAddr base, usize size, VmasFlags flags);

        static auto switch_context(Self *from, Self *to) -> void;

        auto init() -> Status;

        auto map(VirtAddr, PhysAddr, usize n, MmuFlags flags, MapControl control, usize *mapped) -> Status;

        auto map_bulk(VirtAddr, PhysAddr *, usize n, MmuFlags flags, MapControl control, usize *mapped) -> Status;

        auto unmap(VirtAddr, usize n, UnmapControl control, usize *unmapped) -> Status;

        auto protect(VirtAddr, usize n, MmuFlags) -> Status;

        auto query(VirtAddr, ai_out PhysAddr *, ai_out MmuFlags *) -> Status;

        auto mark_accessed(VirtAddr, usize) -> Status
        {  return Status::Unsupported;  }

        auto harvest_accessed(VirtAddr va, usize n, HarvestControl action) -> Status;

        auto active_cpus() const -> CpuMask;
    
    private:
        ArchVmAspace(Self const &) = default;
        ArchVmAspace(Self &&) = default;

        GKTL_CANARY(ArchVmAspace, canary_);

        u16 pcid_;
        VmasFlags flags_;
        /// CPUs that are currently executing in this virtual address space.
        CpuMask active_cpus_;
        gktl::Range<VirtAddr> range_;
        PageTable page_table_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_ARCH_VM_ASPACE_HPP 
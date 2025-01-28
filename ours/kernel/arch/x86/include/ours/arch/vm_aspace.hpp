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

#include <ours/mem/types.hpp>
#include <ours/arch/x86/page_table.hpp>

#include <ours/status.hpp>

#include <gktl/range.hpp>

namespace ours::mem {
    class ArchVmAspace
    {
        typedef ArchVmAspace  Self;

    public:
        ArchVmAspace();

        static auto context_switch(Self *from, Self *to) -> void;

        auto init(gktl::Range<VirtAddr>, u64 flags) -> Status;

        auto map(VirtAddr, usize n, u64 flags) -> Status;

        auto map_to(PhysAddr, VirtAddr, usize n, u64 flags) -> Status;

        auto unmap(VirtAddr, usize n, u64 flags) -> Status;

        auto query(VirtAddr) -> ustl::Option<Pte>;

        auto which_cpu() const -> usize;
    
    private:
        GKTL_CANARY(ArchVmAspace, canary_);

        gktl::Range<VirtAddr> range_;
        IX86PageTable *page_table_;
        // ustl::Variant<PageTable, ExtentPageTable> page_table_storage_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_ARCH_VM_ASPACE_HPP 
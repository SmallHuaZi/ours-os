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
#ifndef OURS_ARCH_X86_PAGE_TABLE_HPP
#define OURS_ARCH_X86_PAGE_TABLE_HPP 1

#include <ours/mem/mod.hpp>
#include <ours/mem/gaf.hpp>
#include <ours/mem/vm_page.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mutex.hpp>

#include <arch/page_table.hpp>

namespace ours::mem::details {
    struct PageAllocator {
        static auto alloc_page(usize nr_pages, usize align) -> PhysAddr {
            PhysAddr phys_addr;
            auto frame = alloc_frame(kGafKernel, &phys_addr, 0); 
            if (!frame) {
                return 0;
            }
            auto page = role_cast<PfRole::Vmm>(frame);
            page->num_mappings += 1;

            return phys_addr;
        }

        static auto free_page(PhysAddr phys_addr, usize nr_pages) -> void {
            if (auto frame = phys_to_frame(phys_addr)) {
                auto page = role_cast<PfRole::Vmm>(frame);
                page->num_mappings -= 1;
                if (0 == page->num_mappings) {
                    free_frame(frame, 0);
                } 
            }
        }
    };

    struct TlbInvalidator {
        static auto invalidate(VirtAddr virt_addr) -> void
        {}
    };

    typedef arch::PageTable
    <
        arch::paging::PageSourceT<PageAllocator>,
        arch::paging::PageFlusherT<TlbInvalidator>,
        arch::MutexT<Mutex>
    > PageTable;

} // namespace ours::mem::details

#endif // #ifndef OURS_ARCH_X86_PAGE_TABLE_HPP
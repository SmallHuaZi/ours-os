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
#ifndef OURS_MEM_VM_COW_PAGES_HPP
#define OURS_MEM_VM_COW_PAGES_HPP 1

#include <ours/mutex.hpp>

#include <ours/mem/gaf.hpp>
#include <ours/mem/vm_page_map.hpp>
#include <ours/mem/page_request.hpp>

#include <ustl/rc.hpp>
#include <ustl/sync/lockguard.hpp>

namespace ours::mem {
    /// A group of the copy on write page.
    class VmCowPages: public ustl::RefCounter<VmCowPages> {
        typedef VmCowPages  Self;
    public:
        class Cursor;

        static auto create(Gaf gaf, usize size, ustl::Rc<VmCowPages> *out) -> Status;

        auto commit_range_locked(VirtAddr offset, usize size, ai_out usize *nr_commited) -> Status;

        auto make_cursor(VirtAddr offset, usize size) -> ustl::Result<Cursor, Status>;

        FORCE_INLINE
        auto size_locked() const -> usize {
            return size_;
        }
    private:
        VmCowPages(Gaf gaf, usize num_pages);

        auto alloc_pages(usize order, VmPage **page, PageRequest *page_request) -> Status;

        /// When no page sources exists, it will be used in frame allocation request.
        Gaf gaf_;
        /// Page map that operates within the virtual memory range of the VMO, from [0, N).
        VmPageMap pagemap_;
        ustl::Rc<PageSource> page_source_;
        usize size_;
    };

    class VmCowPages::Cursor {
    public:
        Cursor(VmCowPages *owner, VirtAddr offset, usize size);

        auto require_owned_page(usize nr_pages, PageRequest *page_request) -> ustl::Result<VmPage *, Status>;

        auto create_read_request(usize nr_pages, PageRequest *page_request) -> Status;
        auto create_dirty_request(usize nr_pages, PageRequest *page_request) -> Status;
    private:
        VmCowPages *owner_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_COW_PAGES_HPP
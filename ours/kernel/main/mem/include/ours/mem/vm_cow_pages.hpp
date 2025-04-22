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

#include <ours/mem/vm_page_list.hpp>
#include <ours/mem/gaf.hpp>
#include <ours/mutex.hpp>

#include <ustl/sync/lockguard.hpp>

namespace ours::mem {
    /// A group of the copy on write page.
    class VmCowPages {
    public:
        class LookupCursor;

        auto commit_range_locked(PgOff pgoff, usize n, ai_out usize *nr_commited) -> Status;

        FORCE_INLINE
        auto num_pages_locked() const -> usize {
            return num_pages_;
        }

    private:
        /// When no page sources exists, it will be used in frame allocation request.
        Gaf gaf_;
        usize num_pages_;
    };

    class VmCowPages::LookupCursor {
    public:
        auto require_owned_page() -> void;
        auto require_readonly_page() -> void;
    private:
        VmPageList pages_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_COW_PAGES_HPP
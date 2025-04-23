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
#ifndef OURS_MEM_PAGE_QUEUES_HPP
#define OURS_MEM_PAGE_QUEUES_HPP 1

#include <ours/mem/vm_page.hpp>

#include <ustl/array.hpp>

namespace ours::mem {
    class PageQueues {
    public:
        CXX11_CONSTEXPR
        static usize const kNumReclaimable = 8;

        auto set_pinned(VmPage *page) -> void;
        auto set_anonymous(VmPage *page) -> void;
        auto set_reclaimable(VmPage *page) -> void;

        auto mark_accessed(VmPage *page) -> void;
    private:
        enum QueueType {
            None,
            Anonymous,
            Pinned,
            ReclaimableStart,
            ReclaimableEnd = ReclaimableStart + kNumReclaimable - 1,
            MaxNumQueues,
        };

        ustl::Array<VmPageList, MaxNumQueues> queues;
        ustl::sync::AtomicU32   lrugen;
        ustl::sync::AtomicU32   mrugen;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PAGE_QUEUES_HPP
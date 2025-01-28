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

#ifndef OURS_MEM_PM_FRAME_HPP
#define OURS_MEM_PM_FRAME_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/pfs.hpp>
#include <ours/mem/constant.hpp>

#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::mem {
    class alignas(64) PmFrame
    {
        typedef PmFrame     Self;
    public:
        auto set_flags(Pfs flags) -> void
        {  this->flags_ |= flags;  }

        auto map_count() const -> usize
        {  return this->mapped_count_;  }

        auto use_count() const -> usize
        {  return this->reference_count_;  }

        auto is_active() const -> bool
        {  return bool(flags_ & Pfs::Active);  }

        auto is_pinned() const -> bool
        {  return bool(flags_ & Pfs::Pinned);  }

    private:
        Pfs flags_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;

        /// Maybe simultaneously held by multi-VmPage.
        mutable ustl::sync::AtomicU32 mapped_count_;
        mutable ustl::sync::AtomicU32 reference_count_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
    };

    USTL_DECLARE_LIST_TEMPLATE(PmFrame, FrameList, PmFrame::ManagedListOptions);

    inline auto operator==(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x == &y;  }

    inline auto operator!=(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x != &y;  }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
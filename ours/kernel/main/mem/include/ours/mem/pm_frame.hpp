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
#include <ours/mem/cfg.hpp>

#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::mem {
    class alignas(64) PmFrame
    {
        typedef PmFrame     Self;
    public:
        auto set_flags(PfStates flags) -> void
        {  this->flags_ |= flags;  }

        auto map_count() const -> usize
        {  return this->mapped_count_;  }

        auto use_count() const -> usize
        {  return this->reference_count_;  }

        auto is_active() const -> bool
        {  return flags_ & PfStates::Active;  }

        auto is_pinned() const -> bool
        {  return flags_ & PfStates::Pinned;  }

        auto is_role(PfRole role) const -> bool
        {  return flags_ & role;  }

        auto role() -> PfRole
        {  return pfns::get_role(flags_);  }

        auto set_role(PfRole role) -> void
        {  return pfns::set_role(flags_, role);  }

        auto zone_type() -> ZoneType 
        {  return pfns::get_zone_type(flags_);  }

        auto set_zone_type(ZoneType type) -> void 
        {  return pfns::set_zone_type(flags_, type);  }

        auto node_id() -> NodeId
        {  return pfns::get_node_id(flags_);  }

    private:
        usize flags_;
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        
        /// Maybe simultaneously held by multi-VmPage.
        mutable ustl::sync::AtomicU32 mapped_count_;
        mutable ustl::sync::AtomicU32 reference_count_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
    };
    USTL_DECLARE_LIST_TEMPLATE(PmFrame, FrameList, PmFrame::ManagedListOptions);

    FORCE_INLINE
    auto operator==(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x == &y;  }

    FORCE_INLINE
    auto operator!=(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x != &y;  }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
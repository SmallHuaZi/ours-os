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
        FORCE_INLINE CXX11_CONSTEXPR
        auto flags() -> FrameFlags & {
            return this->flags_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto flags() const -> FrameFlags const & {
            return this->flags_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto increase_mapping() -> void {
            num_mappings_ += 1;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto decrease_mapping() -> void {
            num_mappings_ += 1;
        }

    private:
        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        
        /// Maybe simultaneously held by multi-VmPage.
        mutable ustl::sync::AtomicU16 num_mappings_;
        mutable ustl::sync::AtomicU16 num_references_;

        FrameFlags flags_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedListOptions);
    };
    USTL_DECLARE_LIST_TEMPLATE(PmFrame, FrameList, PmFrame::ManagedListOptions);
    static_assert(sizeof(PmFrame) <= sizeof(usize) << 8, "");

    FORCE_INLINE
    auto operator==(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x == &y;  }

    FORCE_INLINE
    auto operator!=(PmFrame const &x, PmFrame const &y) -> bool
    {  return &x != &y;  }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
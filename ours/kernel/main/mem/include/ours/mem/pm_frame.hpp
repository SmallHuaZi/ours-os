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
    class alignas(64) PmFrame {
        typedef PmFrame     Self;
    public:
        FORCE_INLINE CXX11_CONSTEXPR
        auto init(ZoneType ztype, SecNum secnum, NodeId nid) -> void {
            flags_.set_zone_type(ztype);
            flags_.set_secnum(secnum);
            flags_.set_nid(nid);
            flags_.set_states(PfStates::Active | PfStates::UpToDate);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto nid() const -> NodeId {
            return flags_.nid();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto zone() const -> ZoneType {
            return flags_.zone_type();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto secnum() const -> SecNum {
            return flags_.secnum();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto order() const -> SecNum {
            return flags_.order();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_order(usize order) -> void {
            flags_.set_order(order);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto reserve() -> void {
            flags_.set_states(PfStates::Reserved);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto role() const -> PfRole {
            return this->flags_.role();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_role(PfRole role) -> void {
            flags_.set_role(role);
        }

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

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_size() const -> usize {
            return BIT(order());
        }

        auto dump() const -> void;

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
    auto operator==(PmFrame const &x, PmFrame const &y) -> bool {
        return &x == &y;
    }

    FORCE_INLINE
    auto operator!=(PmFrame const &x, PmFrame const &y) -> bool {
        return &x != &y;
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
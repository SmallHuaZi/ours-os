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
#include <ours/assert.hpp>

#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/collections/intrusive/list.hpp>
#include <ustl/collections/intrusive/slist.hpp>

namespace ours::mem {
    CXX11_CONSTEXPR
    static auto const kFrameDescSize = sizeof(usize) << 3;

    /// Limit of usage is double word.
    struct PmFrameBase {
        typedef PmFrameBase     Self;

        PmFrameBase() = default;

        /// A frame can not be moved and copied in any way.
        PmFrameBase(Self &&) = delete;
        PmFrameBase(Self const &) = delete;

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
            return flags_.role();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_role(PfRole role) -> void {
            flags_.set_role(role);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_role(PfRole role) const -> bool {
            return flags_.role() == role;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_size() const -> usize {
            return BIT(order());
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mark_folio() -> Self & {
            flags_.set_states(PfStates::Folio);
            return *this;
        }

        auto dump() const -> void;

        auto to_pmm() -> PmFrame * {
            return reinterpret_cast<PmFrame *>(this);
        }

        FrameFlags flags_;
        // The size of following fields type should be hanf of a word.
        mutable ustl::sync::AtomicU32 num_references_;
        // Trace the position of current frame in a large frame.
        mutable ustl::sync::AtomicU32 index_compouned_;
    };

    FORCE_INLINE
    auto operator==(PmFrameBase const &x, PmFrameBase const &y) -> bool {
        return &x == &y;
    }

    FORCE_INLINE
    auto operator!=(PmFrameBase const &x, PmFrameBase const &y) -> bool {
        return &x != &y;
    }

    template <PfRole Role>
    struct RoleViewDispatcher;

    /// Set the role for frame and return the expected role view. 
    template <PfRole Role>
    FORCE_INLINE
    auto role_cast(PmFrameBase &frame) -> RoleViewDispatcher<Role>::Type * {
        typedef typename RoleViewDispatcher<Role>::Type View;
        static_assert(ustl::traits::IsBaseOfV<PmFrameBase, View>);
        static_assert(kFrameDescSize >= sizeof(View));

        frame.set_role(Role);
        return reinterpret_cast<View *>(&frame);
    }

    template <PfRole Role>
    FORCE_INLINE
    auto role_cast(PmFrameBase *frame) -> RoleViewDispatcher<Role>::Type * {
        return role_cast<Role>(*frame);
    }

    /// This is a standard layout frame mainly used by PMM.
    struct alignas(64) PmFrame: public PmFrameBase {
        typedef PmFrame     Self;
        typedef PmFrameBase Base;
        using Base::Base;

        /// Used by PMM
        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedListOptions);
    };
    USTL_DECLARE_LIST_TEMPLATE(PmFrame, FrameList, PmFrame::ManagedListOptions);
    static_assert(sizeof(PmFrame) == kFrameDescSize, "");

    template <>
    struct RoleViewDispatcher<PfRole::Pmm> {
        typedef PmFrame    Type;
    };

    struct MmuFrame: public PmFrameBase {
        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        mutable ustl::sync::AtomicU16 num_mappings;
    };
    static_assert(sizeof(MmuFrame) <= kFrameDescSize, "");

    template <>
    struct RoleViewDispatcher<PfRole::Mmu> {
        typedef MmuFrame    Type;
    };


    struct PmFolio: public PmFrame {
        PmFrameBase second_metadata;
    };

    template <PfRole Role>
    FORCE_INLINE
    auto folio_to_frame(PmFolio *folio) -> RoleViewDispatcher<Role>::Type {
        return static_cast<typename RoleViewDispatcher<Role>::Type * >(folio);
    }

    FORCE_INLINE
    auto frame_to_folio(PmFrameBase *frame) -> PmFolio * {
        return static_cast<PmFolio *>(
            &static_cast<PmFrame *>(frame)[-static_cast<PmFrame *>(frame)->index_compouned_]
        );
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
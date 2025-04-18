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
    static auto const kMaxPmFrameDescSize = sizeof(usize) << 3;

    class PmFrameBase {
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
            return flags_.role();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_role(PfRole role) -> void {
            flags_.set_role(role);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_size() const -> usize {
            return BIT(order());
        }

        auto dump() const -> void;
    private:
        FrameFlags flags_;
        mutable ustl::sync::AtomicU16 num_references_;
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

    /// Before call this, the role of frame was reuiqred to be set explicitly.
    template <PfRole Role>
    FORCE_INLINE
    auto role_cast(PmFrameBase &frame) -> RoleViewDispatcher<Role>::Type * {
        typedef typename RoleViewDispatcher<Role>::Type View;
        static_assert(ustl::traits::IsBaseOfV<PmFrameBase, View>);
        static_assert(sizeof(frame) == sizeof(View));
        frame.set_role(Role);
        return reinterpret_cast<View *>(&frame);
    }

    template <PfRole Role>
    FORCE_INLINE
    auto role_cast(PmFrameBase *frame) -> RoleViewDispatcher<Role>::Type * {
        return role_cast<Role>(*frame);
    }

    /// This is a standard layout frame.
    struct alignas(64) PmFrame: public PmFrameBase {
        typedef PmFrame     Self;
        typedef PmFrameBase Base;
        using Base::Base;

        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedListOptions);
    };
    USTL_DECLARE_LIST_TEMPLATE(PmFrame, FrameList, PmFrame::ManagedListOptions);
    static_assert(sizeof(PmFrame) == sizeof(usize) << 3, "");

    template <>
    struct RoleViewDispatcher<PfRole::Pmm> {
        typedef PmFrame    Type;
    };

    struct MmuFrame: public PmFrameBase {
        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        mutable ustl::sync::AtomicU16 num_mappings;
    };

    template <>
    struct RoleViewDispatcher<PfRole::Mmu> {
        typedef MmuFrame    Type;
    };

    /// When a frame is being used in `ObjectCache`, it's descriptor has the following layout.
    struct SlabFrame: public PmFrameBase {
        typedef SlabFrame   Self;

        FORCE_INLINE CXX11_CONSTEXPR
        auto has_object() const -> bool {
            return num_inuse != num_objects;
        }

        /// Dummy object that provides a way convenient to build the list organizing free objects.
        struct Object: public ustl::collections::intrusive::SlistBaseHook<> {};
        using ObjectList = ustl::collections::intrusive::Slist<Object, 
                           ustl::collections::intrusive::ConstantTimeSize<false>>;

        ObjectCache *object_cache_;
        ustl::sync::AtomicU16 num_inuse;
        ustl::sync::AtomicU16 num_objects;
        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        ObjectList free_list;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedListOptions);
    };
    USTL_DECLARE_LIST_TEMPLATE(SlabFrame, SlabFrameList, SlabFrame::ManagedListOptions);

    template <>
    struct RoleViewDispatcher<PfRole::Slab> {
        typedef SlabFrame    Type;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PM_FRAME_HPP
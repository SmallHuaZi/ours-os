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
#ifndef OURS_MEM_PF_FLAGS_HPP
#define OURS_MEM_PF_FLAGS_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/cfg.hpp>

#include <ustl/bit.hpp>
#include <ustl/bitfields.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
namespace pfns {
    using ustl::TypeList;
    using ustl::Field;
    using ustl::BitFields;
    using ustl::bitfields::Id;
    using ustl::bitfields::Name;
    using ustl::bitfields::Bits;
    using ustl::bitfields::Type;
    using ustl::bitfields::Enable;

    /// `PfStates` is a shorthand of physical frame states.
    enum class PfStates {
        __ActiveBit,
        __PinnedBit,
        __DirtyBit,
        __UpToDateBit,
        __ForeignBit,
        __BeingWaitingBit,
        __FolioBit,
        __ReclaimableBit,
        __ReservedBit,

        Active   = BIT(__ActiveBit),
        Pinned   = BIT(__PinnedBit),
        Dirty    = BIT(__DirtyBit),
        UpToDate = BIT(__UpToDateBit),
        Foreign  = BIT(__ForeignBit),
        BeginWaiting = BIT(__BeingWaitingBit),
        Folio    = BIT(__FolioBit),
        Reclaimable = BIT(__ReclaimableBit),
        Reserved = BIT(__ReservedBit),
        MaxNumStates,
    };
    USTL_ENABLE_ENUM_BITMASK(PfStates);

    enum class PfRole {
        None,
        Io,
        Lru,
        Vmm,
        Pmm,
        Slab,
        Heap,
        MaxNumRoles,
    };

    FORCE_INLINE CXX11_CONSTEXPR
    auto to_string(PfRole role) -> char const * {
        switch (role) {
            case PfRole::None:  return "None";
            case PfRole::Io:    return "Io";
            case PfRole::Lru:   return "Lru";
            case PfRole::Vmm:   return "VMM";
            case PfRole::Pmm:   return "PMM";
            case PfRole::Slab:  return "Slab";
            case PfRole::Heap:  return "Heap";
        }
        return "Anonymous";
    }

    enum PfsFieldId {
        kStateId,
        kRoleId,
        kZoneId,
        kNodeId,
        kOrderId,
        kSectionId,
    };

    typedef TypeList<
        Field<Id<kStateId>, Name<"State">,Type<PfStates>, Bits<BIT_WIDTH(usize(PfStates::MaxNumStates))>>,
        Field<Id<kRoleId>, Name<"Role">, Type<PfRole>, Bits<BIT_WIDTH(usize(PfRole::MaxNumRoles))>>,
        Field<Id<kZoneId>, Name<"Zone">, Type<ZoneType>, Bits<BIT_WIDTH(usize(ZoneType::MaxNumZoneType))>>,
        Field<Id<kNodeId>, Name<"Nid">, Type<NodeId>, Bits<MAX_NODES_BITS>, Enable<OURS_CONFIG_NUMA>>,
        Field<Id<kOrderId>, Name<"Order">, Type<usize>, Bits<BIT_WIDTH(MAX_FRAME_ORDER)>>,
        Field<Id<kSectionId>, Name<"Sec">, Type<SecNum>, Bits<MAX_PHYSADDR_BITS - SECTION_SIZE_BITS>, Enable<1>>
    > FieldList;

    struct FrameFlags {
        FORCE_INLINE CXX11_CONSTEXPR
        auto nid() const -> NodeId {
            return inner_.get<kNodeId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_nid(NodeId nid) -> void {
            inner_.set<kNodeId>(nid);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto zone_type() const -> ZoneType {
            return inner_.get<kZoneId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_zone_type(ZoneType type) -> void {
            inner_.set<kZoneId>(type);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto secnum() const -> SecNum {
            return inner_.get<kSectionId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_secnum(SecNum secnum) -> void {
            inner_.set<kSectionId>(secnum);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto role() const -> PfRole {
            return inner_.get<kRoleId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_role(PfRole role) -> void {
            inner_.set<kRoleId>(role);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_role(PfRole const role) const -> bool {
            return role == inner_.get<kRoleId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto order() const -> usize {
            return inner_.get<kOrderId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_order(usize order) -> void {
            inner_.set<kOrderId>(order);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto state() const -> PfStates {
            return inner_.get<kStateId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_states(PfStates states) -> void {
            inner_.set<kStateId>(inner_.get<kStateId>() | states);
        }

        BitFields<FieldList>    inner_;
    };
    static_assert(sizeof(FrameFlags) <= sizeof(usize), "Never greater than the size target platform supports");

} // namespace ours::mem::pfns
    using pfns::PfStates;
    using pfns::PfRole;
    using pfns::FrameFlags;

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PF_FLAGS_HPP
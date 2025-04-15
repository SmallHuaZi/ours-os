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

    enum PfsFieldId {
        kFrameStateId,
        kFrameRoleId,
        kZoneId,
        kNodeId,
        kOrderId,
        kSectionId,
    };

    /// `PfStates` is a shorthand of physical frame states.
    enum PfStatesBits {
        ActiveBit,
        PinnedBit,
        DirtyBit,
        UpToDateBit,
        ForeignBit,
        BeingWaitingBit,
        FolioBit,
        ReclaimableBit,
        ReservedBit,
        MaxNumStateBits,
    };
    enum class PfStates {
        Active   = BIT(ActiveBit),
        Pinned   = BIT(PinnedBit),
        Dirty    = BIT(DirtyBit),
        UpToDate = BIT(UpToDateBit),
        Foreign  = BIT(ForeignBit),
        BeginWaiting = BIT(BeingWaitingBit),
        Reclaimable = BIT(ReclaimableBit),
        Reserved = BIT(ReservedBit),
    };
    USTL_ENABLE_ENUM_BITMASK(PfStates);

    enum PfRole {
        None,
        Io,
        Lru,
        Mmu,
        Pmm,
        Heap,
        MaxNumRoles,
    };

    typedef TypeList<
        Field<Id<kFrameStateId>, Name<"State">,Type<PfStates>, Bits<BIT_WIDTH(MaxNumStateBits)>>,
        Field<Id<kFrameRoleId>, Name<"Role">, Type<PfRole>, Bits<BIT_WIDTH(MaxNumRoles)>>,
        Field<Id<kZoneId>, Name<"Zone">, Type<ZoneType>, Bits<BIT_WIDTH(MaxNumZoneType)>>,
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
            return inner_.get<kFrameRoleId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_role(PfRole role) -> void {
            inner_.set<kFrameRoleId>(role);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_role(PfRole const role) const -> bool {
            return role == inner_.get<kFrameRoleId>();
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
            return inner_.get<kFrameStateId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_states(PfStates states) -> void {
            inner_.set<kFrameStateId>(states);
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
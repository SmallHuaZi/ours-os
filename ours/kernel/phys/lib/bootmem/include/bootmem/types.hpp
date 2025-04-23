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
#ifndef BOOTMEM_TYPES_HPP
#define BOOTMEM_TYPES_HPP 1

#include <ours/config.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/types.hpp>
#include <ours/status.hpp>
#include <ours/types.hpp>

#include <ustl/bitfields.hpp>

namespace bootmem {
    using ours::i32;
    using ours::isize;
    using ours::PhysAddr;
    using ours::Status;
    using ours::u16;
    using ours::u32;
    using ours::usize;
    using ours::NodeId;
    using ours::mem::Pfn;
    using ours::AlignVal;

    enum class RegionType : u16 {
        Normal,
        Unused,
        Reserved,
        ReservedAndNoInit,
        MaxNumType,
    };

    FORCE_INLINE CXX11_CONSTEXPR 
    auto to_string(RegionType type) -> char const * {
        switch (type) {
            case RegionType::Normal : return "Reserved";
            case RegionType::Reserved: return "Reserved";
            case RegionType::Unused : return "Unused";
            case RegionType::ReservedAndNoInit: return "ReservedAndNoInit";
        }
        return "???";
    }

namespace details {
    using ustl::Field;
    using ustl::BitFields;
    namespace bf = ustl::bitfields;

    struct RegionFlags {
        enum FieldId {
            Type,
            Node,
        };

        FORCE_INLINE CXX11_CONSTEXPR
        auto type() const -> RegionType {
            return inner_.get<Type>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_type(RegionType type) -> void {
            inner_.set<Type>(type);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto nid() const -> NodeId {
            return inner_.get<Node>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_nid(NodeId nid) -> void {
            inner_.set<Node>(nid);
        }

        typedef BitFields<
            bf::Field<bf::Id<Type>, bf::Bits<BIT_WIDTH(u16(RegionType::MaxNumType))>, bf::Name<"Type">, bf::Type<RegionType>>,
            bf::Field<bf::Id<Node>, bf::Bits<MAX_NODES_BITS>, bf::Name<"Nid">, bf::Type<NodeId>>
        > Inner;
        Inner   inner_;
    };
} // namespace details
    using details::RegionFlags;

    enum class AllocationControl {
        BottomUp,
        TopDown = BIT(0),
    };
    USTL_ENABLE_ENUM_BITMASK(AllocationControl);

} // namespace bootmem

#endif // #ifndef BOOTMEM_TYPES_HPP
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

#ifndef BOOTMEM_REGION_HPP
#define BOOTMEM_REGION_HPP 1

#include <ours/config.hpp>
#include <ours/mem/cfg.hpp>
#include <ours/mem/types.hpp>
#include <ours/status.hpp>
#include <ours/types.hpp>

#include <ustl/bit.hpp>
#include <ustl/bitfields.hpp>
#include <ustl/mem/align.hpp>

namespace bootmem {
    using ours::i32;
    using ours::isize;
    using ours::PhysAddr;
    using ours::Status;
    using ours::u16;
    using ours::u32;
    using ours::usize;
    using ours::mem::NodeId;
    using ours::mem::Pfn;

    using ustl::Field;
    using ustl::BitFields;
    using ustl::bitfields::Id;
    using ustl::bitfields::Name;
    using ustl::bitfields::Bits;
    using ustl::bitfields::Type;
    using ustl::bitfields::Enable;

    enum class RegionType : u16 {
        Normal,
        Unused,
        Reserved,
        ReservedAndNoInit,
        MaxNumType,
    };

    struct Region {
        Region() = default;

        Region(PhysAddr base, PhysAddr size, RegionType type, NodeId nid)
            : base(base),
              size(size),
              flags() {
            set_nid(nid);
            set_type(type);
        }

        CXX11_CONSTEXPR
        auto trim(usize align) -> void {
            auto const x = ustl::mem::align_up(base, align);
            size = ustl::mem::align_down(size - (x - base), align);
            base = x;
        }

        CXX11_CONSTEXPR
        auto nid() const -> isize {
            return flags.get<1>();
        }

        CXX11_CONSTEXPR
        auto set_nid(NodeId nid) -> void {
            flags.set<1>(nid);
        }

        CXX11_CONSTEXPR
        auto type() const -> RegionType {
            return flags.get<0>();
        }

        CXX11_CONSTEXPR
        auto set_type(RegionType type) -> void {
            flags.set<0>(type);
        }

        CXX11_CONSTEXPR
        auto end() const -> PhysAddr {
            return base + size;
        }

        PhysAddr base;
        PhysAddr size;

        // clang-format off
        typedef BitFields<
            Field<Id<0>, Bits<BIT_WIDTH(u16(RegionType::MaxNumType))>, Name<"Type">, Type<RegionType>>,
            Field<Id<1>, Bits<MAX_NODES_BITS>, Name<"Nid">, Type<NodeId>>
        > Flags;
        // clang-format on
        Flags flags;
    };

} // namespace bootmem

#endif // #ifndef BOOTMEM_REGION_HPP
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

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/status.hpp>

#include <ours/mem/types.hpp>
#include <ours/mem/constant.hpp>

#include <ustl/bit.hpp>
#include <ustl/mem/align.hpp>

namespace bootmem {
    using ours::Status;
    using ours::u16;
    using ours::i32;
    using ours::u32;
    using ours::isize;
    using ours::usize;
    using ours::PhysAddr;
    using ours::mem::Pfn;
    using ours::mem::NodeId;
    using ours::mem::MAX_NODES;
    using ours::mem::PAGE_SIZE;

    enum class RegionType: u16 {
        Normal,
        Unused,
        Reserved,
        ReservedAndNoInit,
        MaxNumType,
    };

    struct Region
    {
        Region() = default;

        Region(PhysAddr base, usize size, RegionType type, NodeId nid) 
            : base(base), size(size)
        {
            set_nid(nid);
            set_type(type);
        }

        CXX11_CONSTEXPR
        static usize const NID_SHIFT = ustl::bit_width(usize(RegionType::MaxNumType));

        CXX11_CONSTEXPR
        static usize const TYPE_MASK = ((1 << NID_SHIFT) - 1);

        CXX11_CONSTEXPR
        static usize const NID_MASK = ~TYPE_MASK;

        CXX11_CONSTEXPR
        auto trim(usize align) -> void
        {
            auto const x = ustl::mem::align_up(base, align);
            size = ustl::mem::align_down(size - (x - base), align);
            base = x;
        }

        CXX11_CONSTEXPR
        auto nid() const -> isize
        {  return (flags & NID_MASK) >> NID_SHIFT;  }

        CXX11_CONSTEXPR
        auto set_nid(NodeId nid) -> void
        {  flags = (nid << NID_SHIFT) | usize(type());  }

        CXX11_CONSTEXPR
        auto type() const -> RegionType
        {  return RegionType(flags & TYPE_MASK);  }

        CXX11_CONSTEXPR
        auto set_type(RegionType type) -> void
        {  flags = usize(type) | nid();  }

        CXX11_CONSTEXPR
        auto end() const -> usize
        {  return base + size;  }

        PhysAddr base;
        usize size;
        usize flags;
    };

} // namespace bootmem

#endif // #ifndef BOOTMEM_REGION_HPP
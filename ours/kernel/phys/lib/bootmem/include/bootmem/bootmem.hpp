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

#ifndef BOOTMEM_ALLOCATOR_HPP
#define BOOTMEM_ALLOCATOR_HPP 1

#include <bootmem/region.hpp>

#include <ours/mem/types.hpp>

#include <ustl/option.hpp>
#include <gktl/range.hpp>

namespace bootmem {
    struct IterationContext
    {
        CXX11_CONSTEXPR
        IterationContext(NodeId nid, RegionType type, PhysAddr start, PhysAddr end)
            : nid_(nid), type_(type), start_(start), end_(end)
        {}

    private:
        NodeId nid_;
        RegionType type_;
        PhysAddr start_;
        PhysAddr end_;
        Region const *current_;
    };

    class IBootMem
    {
    public:
        enum AllocationControl: usize {
            /// Allocate memory from low address to high address if set
            BottomUp    = BIT(0),
        };

        virtual auto add(PhysAddr base, usize size, RegionType type, NodeId nid) -> void = 0;

        virtual auto remove(PhysAddr base, usize size) -> void = 0;

        virtual auto reserve(PhysAddr base, usize size) -> void = 0;

        virtual auto allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> PhysAddr = 0;

        auto allocate(usize size, usize align, NodeId nid = MAX_NODES) -> PhysAddr
        {  return allocate_bounded(size, align, default_lowest_limit_, default_highest_limit_, nid);  }

        virtual auto free(PhysAddr base, usize size) -> PhysAddr = 0;

        virtual auto iterate(IterationContext &context) const -> ustl::Option<Region> = 0;

        auto start_address() const -> PhysAddr
        {  return start_address_;  }

        auto end_address() const -> PhysAddr
        {  return end_address_;  }

        auto get_range_for_nid(NodeId nid) const -> gktl::Range<PhysAddr>;

        auto get_pfn_range_for_nid(NodeId nid) const -> gktl::Range<Pfn>;

        auto count_present_blocks(PhysAddr start, PhysAddr end, usize bsize, usize balign, NodeId nid) const -> usize;

        auto count_present_frames(PhysAddr start, PhysAddr end, NodeId nid) const -> usize
        {  return count_present_blocks(start, end, PAGE_SIZE, PAGE_SIZE, nid);  }
    private:
        PhysAddr start_address_;
        PhysAddr end_address_;
        PhysAddr default_lowest_limit_;
        PhysAddr default_highest_limit_;
        AllocationControl allocation_control_;
    };

    FORCE_INLINE
    auto IBootMem::get_pfn_range_for_nid(NodeId nid) const -> gktl::Range<Pfn>
    {
        auto [start, end] = get_range_for_nid(nid);
        return { ours::mem::phys_to_pfn(start), ours::mem::phys_to_pfn(end) };
    }

} // namespace bootmem

#endif // #ifndef BOOTMEM_ALLOCATOR_HPP
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

#include <ustl/limits.hpp>
#include <ustl/option.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/traits/is_invocable.hpp>

#include <gktl/range.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(COND, ...)
#endif

#ifndef UNREACHABLE
#   define UNREACHABLE(...) while(1)
#endif

namespace bootmem {
    class IBootMem;

    struct IterationContext
    {
        typedef IterationContext    Self;

        CXX11_CONSTEXPR
        IterationContext(IBootMem const *bootmem, RegionType type)
            : IterationContext(bootmem, MAX_NODES, type, 0, ustl::NumericLimits<usize>::max())
        {}

        CXX11_CONSTEXPR
        IterationContext(IBootMem const *bootmem, NodeId nid, RegionType type)
            : IterationContext(bootmem, nid, type, 0, ustl::NumericLimits<usize>::max())
        {}

        CXX11_CONSTEXPR
        IterationContext(IBootMem const *bootmem, NodeId nid, RegionType type, PhysAddr start, PhysAddr end);

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_nid(NodeId nid) -> Self &
        {
            nid_ = nid;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_type(RegionType type) -> Self &
        {
            type_ = type;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_limit(PhysAddr start, PhysAddr end) -> Self &
        {
            range_.start = start;
            range_.end = end;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_range_available(PhysAddr start, PhysAddr end) -> bool
        {  return range_.overlaps(start, end) ;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto reset() -> void;

        Region *free_;
        Region *used_;
        NodeId nid_;
        RegionType type_;
        gktl::Range<PhysAddr> range_;
    };

    struct IBootMem
    {
        enum AllocationControl: usize {
            TopDown,
            /// Allocate memory from low address to high address if set
            BottomUp    = BIT(0),
        };

        IBootMem()
            : default_lowest_limit_(0),
              default_highest_limit_(ustl::NumericLimits<PhysAddr>::max()),
              start_address_(ustl::NumericLimits<PhysAddr>::max()),
              end_address_(0),
              allocation_control_(TopDown)
        {}

        virtual ~IBootMem() = default;

        virtual auto name() const -> char const * = 0;

        virtual auto add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status = 0;

        virtual auto remove(PhysAddr base, usize size) -> void = 0;

        virtual auto protect(PhysAddr base, usize size) -> Status = 0;

        /// Allocate the `size` bytes of memory aligned to `align` in range from `start` to `end`.
        /// Ensure the memory is matched to the designated node to the best of our ability.
        virtual auto allocate_bounded(usize, usize, PhysAddr, PhysAddr, NodeId = MAX_NODES) -> PhysAddr = 0;

        auto allocate(usize size, usize align, NodeId nid = MAX_NODES) -> PhysAddr
        {  return allocate_bounded(size, align, default_lowest_limit_, default_highest_limit_, nid);  }

        virtual auto deallocate(PhysAddr base, usize size) -> void = 0;

        virtual auto iterate(IterationContext &context) const -> ustl::Option<Region> = 0;

        virtual auto set_node(PhysAddr base, PhysAddr size, NodeId nid) -> void = 0;

        auto count_present_blocks(PhysAddr start, PhysAddr end, usize bsize, usize balign, NodeId nid) const -> usize;

        auto start_address() const -> PhysAddr
        {  return start_address_;  }

        auto end_address() const -> PhysAddr
        {  return end_address_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_allocation_bounds(PhysAddr start, PhysAddr end) -> void {
            default_lowest_limit_ = start;
            default_highest_limit_ = end;
        }

        template <typename F>
            requires ustl::traits::Invocable<F, usize, usize>
        auto find_if(F &&matcher, usize size, usize align, RegionType type, NodeId nid = MAX_NODES) -> PhysAddr
        {
            IterationContext context{this, nid, type, start_address_, end_address_};
            while (auto region = this->iterate(context)) {
                auto const base_aligned = ustl::mem::align_up(region->base, align);
                auto const size_aligned = region->size - (base_aligned - region->base);
                if (size_aligned < size) {
                    continue;
                } else if (matcher(base_aligned, size_aligned)) {
                    // Must return the original region to prevent those requests which 
                    // demands the address aligned to a super alignment value and to avoid
                    // manage so many little blocks.
                    return region->base;
                }
            }

            return 0;
        }

    protected:
        friend IterationContext;
        virtual auto build_iteration_context(IterationContext &context) const -> void = 0;

        PhysAddr start_address_;
        PhysAddr end_address_;
        PhysAddr default_lowest_limit_;
        PhysAddr default_highest_limit_;
        AllocationControl allocation_control_;
    };

    FORCE_INLINE CXX11_CONSTEXPR
    IterationContext::IterationContext(IBootMem const *bootmem, NodeId nid, RegionType type, PhysAddr start, PhysAddr end)
        : nid_(nid), type_(type), range_(start, end) 
    {  bootmem->build_iteration_context(*this);  }

} // namespace bootmem

#endif // #ifndef BOOTMEM_ALLOCATOR_HPP
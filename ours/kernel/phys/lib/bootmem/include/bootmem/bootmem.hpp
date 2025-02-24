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
    struct IterationContext
    {
        typedef IterationContext    Self;

        CXX11_CONSTEXPR
        IterationContext() = default;

        CXX11_CONSTEXPR
        IterationContext(NodeId nid, RegionType type)
            : nid_(nid), type_(type), start_(0), size_(0)
        {}

        CXX11_CONSTEXPR
        IterationContext(NodeId nid, RegionType type, PhysAddr start, PhysAddr end)
            : nid_(nid), type_(type), start_(start), size_(end - start)
        {}

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
            start_ = start;
            size_ = end - start;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_fit(Region const &region) -> bool
        {
            return (region.type() == type_ || type_ == RegionType::AllType) &&
                    region.nid() == nid_ &&
                    region.base > start_ &&
                    region.size < size_; 
        }

        NodeId nid_;
        RegionType type_;
        PhysAddr start_;
        usize size_;
        Region const *free_;
        Region const *used_;
    };

    class IBootMem
    {
    public:
        enum AllocationControl: usize {
            /// Allocate memory from low address to high address if set
            BottomUp    = BIT(0),
        };

        virtual auto name() const -> char const * = 0;

        virtual auto add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status = 0;

        virtual auto remove(PhysAddr base, usize size) -> void = 0;

        virtual auto protect(PhysAddr base, usize size) -> Status = 0;

        virtual auto allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> PhysAddr = 0;

        auto allocate(usize size, usize align, NodeId nid = MAX_NODES) -> PhysAddr
        {  return allocate_bounded(size, align, default_lowest_limit_, default_highest_limit_, nid);  }

        virtual auto deallocate(PhysAddr base, usize size) -> void = 0;

        virtual auto iterate(IterationContext &context) const -> ustl::Option<Region> = 0;

        auto start_address() const -> PhysAddr
        {  return start_address_;  }

        auto end_address() const -> PhysAddr
        {  return end_address_;  }

        auto count_present_blocks(PhysAddr start, PhysAddr end, usize bsize, usize balign, NodeId nid) const -> usize;

        template <typename F>
            requires ustl::traits::Invocable<F, usize, usize>
        auto find_if(F &&matcher, usize size, usize align, RegionType type, NodeId nid = MAX_NODES) -> PhysAddr 
        {
            IterationContext context{nid, type, start_address_, end_address_};
            while (auto region = this->iterate(context)) {
                auto const base_aligned = ustl::mem::align_up(region->base, align);
                auto const size_aligned = region->size - (base_aligned - region->base);
                if (size_aligned < size || type != region->type()) {
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

    private:
        PhysAddr start_address_;
        PhysAddr end_address_;
        PhysAddr default_lowest_limit_;
        PhysAddr default_highest_limit_;
        AllocationControl allocation_control_;
    };

} // namespace bootmem

#endif // #ifndef BOOTMEM_ALLOCATOR_HPP
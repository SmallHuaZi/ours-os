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
#ifndef BOOTMEM_MEMBLOCK_HPP
#define BOOTMEM_MEMBLOCK_HPP 1

#include <bootmem/bootmem.hpp>

#include <ustl/option.hpp>
#include <ustl/util/pair.hpp>
#include <ustl/fmt/formatter.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/collections/pmr/vec.hpp>
#include <ustl/algorithms/copy.hpp>

namespace bootmem {
    struct MemBlock
        : public IBootMem
    {
        struct RegionList
        {
            typedef Region const* Iter;
            typedef Region *      IterMut;

            RegionList(MemBlock *holder)
                : holder_(holder),
                  count_(0),
                  capacity_(0)
            {}

            auto add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status;
            auto remove(PhysAddr base, usize size) -> Status;
            auto isolate(PhysAddr base, usize size) -> ustl::Pair<IterMut, IterMut>;
            auto reset(Region *new_region, usize new_capacity) -> void;

            auto begin() const -> Iter
            {  return regions_;  }

            auto end() const -> Iter
            {  return regions_ + count_;  }

            auto begin() -> IterMut
            {  return regions_;  }

            auto end() -> IterMut
            {  return regions_ + count_;  }

            auto size() const -> usize 
            {  return count_;  }

            auto capacity() const -> usize 
            {  return capacity_;  }

            template <typename F>
            auto grow(usize new_capacity, F &&f) -> bool;

            auto grow(usize new_capacity) -> bool;

            auto grow(usize new_capacity, PhysAddr reserved_start, PhysAddr reserved_end) -> bool;

            auto erase(IterMut pos) -> void
            { 
                ustl::algorithms::copy(pos + 1, end(), pos); 
                count_ -= 1;
            }

            auto erase(IterMut first, IterMut last) -> void
            {
                if (last != end()) {
                    ustl::algorithms::copy(last, end(), first); 
                }
                count_ -= last - first;
            }

            auto at(usize index) const -> Region & {
                if (index < count_) {
                    return regions_[index];
                }
                UNREACHABLE();
            }

            auto insert(usize index, Region const &region) -> void;

            auto push_back(Region const &region) -> void;

            auto try_merge(usize first, usize last) -> Status;

            static auto calc_new_capacity(usize old_capacity) -> usize;

            MemBlock *holder_;
            Region *regions_;
            usize capacity_;
            usize count_;
            usize total_;
        };
    public:
        MemBlock()
            : IBootMem(),
              memories_(this),
              reserved_(this)
        {}

        virtual ~MemBlock() override
        {}

        auto init(Region *new_regions, usize n) -> void;

        auto trim(usize align) -> void;

        auto name() const -> char const * override
        {  return "memblock";  }

        auto add(PhysAddr base, usize size, RegionType type, NodeId nid = MAX_NODES) -> Status override
        { 
            start_address_ = ustl::algorithms::min(base, start_address_);
            end_address_ = ustl::algorithms::max(base + size, end_address_);
            return memories_.add(base, size, type, nid); 
        }

        auto remove(PhysAddr base, usize size) -> void override
        {  memories_.remove(base, size);  }

        auto protect(PhysAddr base, usize size) -> Status override
        {  return reserved_.add(base, size, RegionType::Normal, 0);  }

        auto allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId = MAX_NODES) -> PhysAddr override;

        auto deallocate(PhysAddr ptr, usize size) -> void override;

        auto iterate(IterationContext &context) const -> ustl::Option<Region> override;

        auto set_node(PhysAddr base, PhysAddr size, NodeId nid) -> void override;

        // The following methods was marked public just for tests
        auto memories_list() -> RegionList &
        {  return memories_;  }

        auto reserved_list() -> RegionList &
        {  return reserved_;  }
    private:
        auto build_iteration_context(IterationContext &context) const -> void override;
        auto iterate_unused_region(IterationContext &context) const -> ustl::Option<Region>;

        RegionList   memories_;
        RegionList   reserved_;
    }; // class MemBlock

} // namespace bootmem

// template <typename Char>
// template <>
// class ustl::fmt::Formatter<bootmem::MemBlock, char>
// class std::formatter<bootmem::MemBlock, char>
// {
//     typedef formatter          Self;
//     typedef bootmem::MemBlock  FormattedObject;

// public:
//     template <typename ParseContext>
//     auto parse(ParseContext &) const -> typename ParseContext::IterMut;

//     template <typename FormatContext>
//     auto format(FormattedObject const &, FormatContext &) const -> typename FormatContext ::IterMut;

//     template <typename FormatContext>
//     auto formatted_size(FormattedObject const &, FormatContext &) const -> ustl::usize;

// }; // class Formatter<Memblock>

#endif // #ifndef BOOTMEM_MEMBLOCK_HPP

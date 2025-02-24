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
    class MemBlock
        : public IBootMem
    {
        struct RegionList
        {
            typedef Region const* Iter;
            typedef Region *      IterMut;

            RegionList(MemBlock *holder)
                : holder_(holder)
            {}

            auto add(PhysAddr base, usize size, RegionType type, NodeId nid) -> Status;
            auto remove(PhysAddr base, usize size) -> Status;
            auto isolate(PhysAddr base, usize size) -> ustl::Pair<IterMut, IterMut>;

            auto begin() const -> Iter
            {  return regions_;  }

            auto end() const -> Iter
            {  return regions_ + count_;  }

            auto begin() -> IterMut
            {  return regions_;  }

            auto end() -> IterMut
            {  return regions_ + count_;  }

            auto grow(usize capacity) -> bool;

            auto erase(IterMut pos) -> void
            { ustl::algorithms::copy(pos + 1, end(), pos); }

            auto erase(IterMut first, IterMut last) -> void
            {
                if (last == end()) {
                    count_ -= last - first;
                } else {
                    ustl::algorithms::copy(last + 1, end(), first); 
                }
            }

            auto insert(usize index, Region const &region) -> void;

            auto push_back(Region const &region) -> void;

            auto try_merge(usize first, usize last) -> Status;

            MemBlock *holder_;
            Region *regions_;
            usize capacity_;
            usize count_;
            usize total_;
        };
    public:
        MemBlock()
            : memories_(this),
              reserved_(this)
        {}

        auto init(PhysAddr base, usize size) -> void;

        auto name() const -> char const * override
        {  return "memblock";  }

        auto add(PhysAddr base, usize size, RegionType type, NodeId nid = MAX_NODES) -> Status override
        { return memories_.add(base, size, type, nid); }

        auto remove(PhysAddr base, usize size) -> void override
        { memories_.remove(base, size); }

        auto protect(PhysAddr base, usize size) -> Status override
        {  return reserved_.add(base, size, RegionType::AllType, 0);  }

        auto allocate_bounded(usize size, usize align, PhysAddr start, PhysAddr end, NodeId nid) -> PhysAddr override;

        auto deallocate(PhysAddr ptr, usize size) -> void override;

        auto iterate(IterationContext &context) const -> ustl::Option<Region> override;
    private:
        RegionList   memories_;
        RegionList   reserved_;
    }; // class MemBlock

} // namespace bootmem

template <typename Char>
class ustl::fmt::Formatter<bootmem::MemBlock, Char>
{
    typedef Formatter          Self;
    typedef bootmem::MemBlock  FormattedObject;

public:
    template <typename ParseContext>
    auto parse(ParseContext &) const -> typename ParseContext::IterMut;

    template <typename FormatContext>
    auto format(FormattedObject const &, FormatContext &) const -> typename FormatContext ::IterMut;

    template <typename FormatContext>
    auto formatted_size(FormattedObject const &, FormatContext &) const -> usize;

}; // class Formatter<Memblock>

#endif // #ifndef BOOTMEM_MEMBLOCK_HPP

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
#include <ustl/traits/is_invocable.hpp>
#include <ustl/collections/pmr/vec.hpp>

namespace bootmem {
    struct MemBlock;
    struct RegionList;

    struct BootstrapMemoryResource
        : public ustl::collections::pmr::MemoryResource
    {
        virtual auto do_allocate(usize bytes, usize align) -> void *
        {  return 0;  }

        virtual auto do_deallocate(void *p, usize bytes, usize align) -> void
        {  return; }

        constexpr static usize const NR_INITIAL_REGIONS = 8;

        auto allocate(usize n, usize alignment) -> Region *;

        auto deallocate(Region* p, usize n) -> void;

        auto commit_booking() -> void;

        auto book(usize base, usize size) -> void;

        MemBlock *memblock_;
        bool should_commit_;
        usize booking_base_;
        usize booking_size_;
        Region initial_regions_[NR_INITIAL_REGIONS];
    };

    class RegionList
    {
        using Self = RegionList;
        using Inner = ustl::collections::pmr::Vec<Region>;

    public:
        typedef Inner::Iter         Iter;
        typedef Inner::IterMut      IterMut;

        RegionList(char const *name, BootstrapMemoryResource &allocator)
            : inner_(BootstrapMemoryResource::NR_INITIAL_REGIONS, allocator),
              total_(0),
              name_(name)
        {}

        auto add(usize base, usize size, flags flags = {}) -> Status;

        auto remove(usize base, usize size) -> Status;

        auto isolate(usize base, usize size) -> ustl::Pair<IterMut, IterMut>;

        auto begin() -> IterMut
        {  return inner_.begin();  }

        auto end() -> IterMut
        {  return inner_.end();  }
    
    private:
        auto insert(usize index, usize base, usize size, Flags flags) -> void;

        auto push_back(usize base, usize size, Flags flags) -> void;

        auto try_merge(usize first, usize last) -> Status;

        Inner inner_;
        usize total_;
        char const *name_;
    };

    class MemBlock
        : public IBootMem
    {
        typedef MemBlock   Self;
    public:
        MemBlock(BootstrapMemoryResource *bootstrap)
            : available_list_("Available memory", *memory_resource_),
              occupied_list_("Occupied memory", *memory_resource_),
              memory_resource_(bootstrap),
              default_lowest_limit_(0),
              default_uppest_limit_(SIZE_MAX)
        {  bootstrap->memblock_ = this;  }

        auto add(usize base, usize size, Flags flags = NO_FLAGS) -> Status
        { return available_list_.add(base, size, flags); }

        auto remove(usize base, usize size) -> Status
        { return available_list_.remove(base, size); }

        auto protect(usize base, usize size, Flags flags = NO_FLAGS) -> Status
        {  return occupied_list_.add(base, size, flags);  }

        auto allocate_bounded(usize size, usize align, usize lower_limit, usize upper_limit) -> usize;

        auto allocate(usize size, usize align) -> usize
        {  return allocate_bounded(size, align, default_lowest_limit_, default_uppest_limit_);  }

        auto deallocate(void *ptr, usize size) -> void;

        struct Cursor
        {
            Cursor(MemBlock *memblock)
                : memblock_(memblock),
                  free_iter_(memblock_->available_list_.begin()),
                  busy_iter_(memblock_->occupied_list_.begin())
            {}

            auto move_next() -> ustl::Option<Region>;

            auto move_prev() -> ustl::Option<Region>;

            auto reset() -> void
            {
                free_iter_ = memblock_->available_list_.begin();
                busy_iter_ = memblock_->occupied_list_.begin();
            }

            MemBlock *memblock_;
            RegionList::IterMut free_iter_;
            RegionList::IterMut busy_iter_;
        };

        template <typename F>
        auto find_if(F &&matcher, usize size, usize align, Flags flags = NO_FLAGS) -> usize
        {
            Cursor cursor{this};
            while (auto region = cursor.move_next()) {
                auto const base_aligned = ustl::mem::align_up(region->base, align);
                auto const size_aligned = region->size - (base_aligned - region->base);
                if (size_aligned < size || flags != region->flags) {
                    continue;
                } else if (matcher(base_aligned, size_aligned, flags)) {
                    return base_aligned;
                }
            }

            return 0;
        }

        template <typename F>
        constexpr static bool InvocableWithBlock = ustl::traits::Invocable<F, usize, usize, Flags>;

        template <typename F>
            requires InvocableWithBlock<F>
        auto for_each_all_block(F &&functor) -> void
        {}

        template <typename F>
            requires InvocableWithBlock<F>
        auto for_each_available_block_in_range(usize base, usize size, F &&functor) -> void;

        template <typename F>
            requires InvocableWithBlock<F>
        auto for_each_occupied_block_in_range(usize base, usize size, F &&functor) -> void;

    private:
        BootstrapMemoryResource *memory_resource_;
        bool alloc_from_bottom_to_up_;
        usize default_lowest_limit_;
        usize default_uppest_limit_;
        RegionList   available_list_;
        RegionList   occupied_list_;
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

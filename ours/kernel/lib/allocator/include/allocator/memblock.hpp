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

#ifndef ALLOCATOR_MEMBLOCK_HPP
#define ALLOCATOR_MEMBLOCK_HPP 1

#include <ustl/util/pair.hpp>
#include <ustl/fmt/formatter.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/collections/pmr/vec.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(COND, ...)
#endif

namespace allocator {
    enum Status {
        Ok,
        Fail,
        OutOfMemory,
        AlreadyExisting,
        InvalidRange,
    };

    typedef size_t      Flags;

    struct MemRegion {
        size_t base;
        size_t size;
        Flags flags;

        constexpr auto end() const -> size_t
        {  return base + size - 1; }
    };

    class MemRegionList
    {
        typedef MemRegionList                               Self;
        typedef ustl::collections::pmr::Vec<MemRegion>      Inner;

    public:
        typedef Inner::Iter         Iter;
        typedef Inner::IterMut      IterMut;

        MemRegionList();

        auto add(size_t base, size_t size, Flags flags = {}) -> Status;

        auto remove(size_t base, size_t size) -> Status;

        auto isolate(size_t base, size_t size) -> Status;

        enum ContainsResult {
            /// [ () ]
            Included,
            /// [] () 
            Exclusive,
            /// [ (][) ]
            WithoutCenter,
            /// [ (] )
            WithoutLeft,
            /// ( [) ]
            WithoutRight,
        };
        auto contains(size_t *base, size_t *size) -> ContainsResult;

        auto begin() -> IterMut
        {  return inner_.begin();  }

        auto end() -> IterMut
        {  return inner_.end();  }
    
    private:
        auto insert(size_t index, size_t base, size_t size, Flags flags) -> Status;

    private:
        Inner inner_;
        size_t total_;
        char const *name_;
    };

    class MemBlock 
    {
        typedef MemBlock   Self;
    public:
        MemBlock();

        auto add(size_t base, size_t size, size_t flags = 0) -> Status
        { return available_list_.add(base, size, flags); }

        auto remove(size_t base, size_t size) -> Status
        { return available_list_.remove(base, size); }

        enum class ContainsResult {
            All,
            Part,
            Exclude,
        };
        auto contains(size_t base, size_t size, size_t flags = 0) -> ContainsResult;

        auto protect(size_t base, size_t size) -> Status;

        auto allocate(size_t size, size_t align) -> void *;

        auto allocate_bounded(size_t size, size_t align, size_t lower_limit, size_t upper_limit) -> void *;

        auto deallocate(void *ptr, size_t size, size_t align) -> void;

        auto find(size_t base, size_t size, size_t flags = 0) -> ustl::Pair<size_t, size_t>;

        template <typename F>
            requires ustl::traits::Invocable<F, size_t, size_t>
        auto for_each_all_block(F &&functor) -> void;

        template <typename F>
            requires ustl::traits::Invocable<F, size_t, size_t>
        auto for_each_available_block_in_range(size_t base, size_t size, F &&functor) -> void;

        template <typename F>
            requires ustl::traits::Invocable<F, size_t, size_t>
        auto for_each_occupied_block_in_range(size_t base, size_t size, F &&functor) -> void;

    private:
        auto check_and_rectify_range(size_t *base, size_t *size, size_t flags) -> Status;

    // private:
    //     auto find(size_t size, size_t align, size_t lower_limit, size_t upper_limit) -> MemRegion *;
    //     static auto merge(BlockList &list) -> void;
    //     static auto isolate(RegionList &list, size_t base, size_t size) -> ustl::Pair<Iter, Iter>;
    //     static auto insert_and_merge(RegionList &list, MemRegion const &section) -> void;

    private:
        struct Cursor;
        MemRegionList available_list_;
        MemRegionList occupied_list_;

    }; // class MemBlock

} // namespace allocator 

template <typename Char>
class ustl::fmt::Formatter<allocator::MemBlock, Char>
{
    typedef Formatter              Self;
    typedef allocator::MemBlock    FormattedObject;

public:
    template <typename ParseContext>
    auto parse(ParseContext &) const -> ParseContext::IterMut;

    template <typename FormatContext>
    auto format(FormattedObject const &, FormatContext &) const -> FormatContext ::IterMut;

    template <typename FormatContext>
    auto formatted_size(FormattedObject const &, FormatContext &) const -> size_t;

}; // class Formatter<Memblock>

#endif // #ifndef ALLOCATOR_MEMBLOCK_HPP
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

#include <bootmem/algorithms.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/algorithms/copy.hpp>
#include <ustl/iterator/function.hpp>
#include <ustl/iterator/reverse_iterator.hpp>
#include <ustl/option.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/option.hpp>
#include <ustl/util/pair.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

namespace bootmem {
    template <template <typename> typename Collection>
    class MemBlock;

    template <template <typename> typename Collection>
    struct RegionList
        : protected Collection<Region>
    {
        typedef Collection<Region>  Base;
        typedef typename Base::Iter     Iter;
        typedef typename Base::IterMut  IterMut;

        template <template <typename> typename>
        friend class MemBlock;

        using Base::begin;
        using Base::end;
        using Base::rbegin;
        using Base::rend;
        using Base::size;
        using Base::capacity;
        using Base::reserve;
        using Base::Base;
        using Base::reset;

        RegionList() = default;

        RegionList(Collection<Region> &&collection)
            : Base(ustl::move(collection))
        {}

        CXX11_CONSTEXPR
        auto add(PhysAddr base, usize size, RegionType type = RegionType::Normal, NodeId nid = MAX_NODES) -> Status {
            if (Base::size() == 0) {
                emplace_back(base, size, type, nid);
                return Status::Ok;
            }
            auto const end = base + size;

            IterMut iter = this->begin(), first = this->end(), last;
            for (; iter != this->end(); ++iter) {
                auto const rbase = iter->base;
                auto const rend = iter->end();

                if (rbase >= end) {
                    break;
                } else if (rend <= base) {
                    continue;
                }

                // Handle the intersection.
                if (rbase > base) {
                    if (first == this->end()) {
                        first = iter;
                    }
                    last = iter + 1;
                    Base::emplace(iter++, base, rbase - base, type, nid);
                }

                base = min(rend, end);
            }

            if (base < end) {
                if (first == this->end()) {
                    first = iter;
                }
                last = iter + 1;
                Base::emplace(iter, base, end - base, type, nid);
            }

            if (first != this->end()) {
                if (first != this->begin()) {
                    --first;
                }
                try_merge(first, last);
            }

            return Status::Ok;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto remove(PhysAddr base, PhysAddr size) -> void {
            auto maybe_range = isolate(base, size);
            if (!maybe_range) {
                return;
            }
            erase(maybe_range->first, maybe_range->second);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto trim(usize alignment) -> void {
            auto iter = begin();
            while (iter != end()) {
                iter->trim(alignment);
                if (iter->size == 0) {
                    iter = erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto size_bytes() const -> usize {
            return total_bytes_;
        }
    private:
        using Base::emplace;
        using Base::erase;

        template <typename... Args>
        FORCE_INLINE CXX11_CONSTEXPR
        auto emplace(Iter pos, Args &&...args) -> IterMut {
            auto iter = Base::emplace(pos, ustl::forward<Args>(args)...);
            total_bytes_ += iter->size;
            return iter;
        }

        template <typename... Args>
        FORCE_INLINE CXX11_CONSTEXPR
        auto emplace_back(Args &&...args) -> void {
            Base::emplace_back(ustl::forward<Args>(args)...);
            total_bytes_ += Base::back().size;
        }

        CXX11_CONSTEXPR
        auto erase(Iter const &first, Iter const &last) -> void {
            for (auto i = first; i != last; ++i) {
                total_bytes_ -= i->size;
            }
            Base::erase(first, last);
        }

        CXX11_CONSTEXPR
        auto erase(Iter const &pos) -> void {
            total_bytes_ -= pos->size;
            Base::erase(pos);
        }

        CXX11_CONSTEXPR
        auto isolate(PhysAddr base, PhysAddr size) -> ustl::Option<ustl::Pair<IterMut, IterMut>> {
            if (Base::size() + 2 > capacity()) {
                reserve(calc_new_capacity(2));
            }

            bool isolated = false;
            IterMut first, last = Base::begin();
            auto const end = base + size;
            for (auto iter = Base::begin(); iter != Base::end(); ++iter) {
                auto const rbase = iter->base;
                auto const rend = iter->end();
                auto const type = iter->type();
                auto const nid = iter->nid();

                if (rbase >= end) {
                    break;
                } else if (rend <= base) {
                    continue;
                } else if (rbase < base) { // The code bottom will handle the intersection
                    // Rectifies this region to [rbase, base)
                    auto const left_diff_size = base - rbase;
                    iter->base = base;
                    iter->size -= left_diff_size;
                    total_bytes_ -= left_diff_size;
                    emplace(iter, rbase, left_diff_size, type, nid);
                } else if (rend > end) {
                    // Rectifies this region to [end, rend)
                    auto const right_diff_size = end - rbase;
                    iter->base = end;
                    iter->size -= right_diff_size;
                    total_bytes_ -= right_diff_size;
                    emplace(iter--, rbase, right_diff_size, type, nid);
                } else {
                    if (!isolated) {
                        isolated = true;
                        first = iter;
                    }
                    last = iter + 1;
                }
            }

            if (!isolated) {
                return ustl::none();
            }

            return ustl::make_pair(first, last);
        }

        CXX11_CONSTEXPR
        auto try_merge(IterMut first, IterMut last) -> void {
            while (first != last) {
                IterMut next = first + 1;
                if (next == last) {
                    break;
                }

                if (first->end() != next->base ||
                    first->nid() != next->nid() ||
                    first->type() != next->type()) {
                    ++first;
                    continue;
                }

                first->size += next->size;
                erase(next);
                --last;
            }
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto calc_new_capacity(usize additional = 0) const -> usize {
            auto const old_capacity = capacity();
            return ustl::algorithms::max(additional + old_capacity, old_capacity << 1);
        }

        usize total_bytes_;
    };

    template <typename>
    struct RegionVector;

    template <template <typename> typename Collection = RegionVector>
    class MemBlock {
        typedef MemBlock   Self;

        template <bool Mutable>
        struct NormalIterator;
    public:
        typedef NormalIterator<false>       Iter;
        typedef NormalIterator<true>        IterMut;
        typedef RegionList<Collection>      RegionList;

        FORCE_INLINE CXX11_CONSTEXPR
        MemBlock()
            : MemBlock(Collection(), Collection())
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        MemBlock(Collection<Region> &&memories, Collection<Region> &&reserved)
            : memories_(ustl::move(memories)),
              reserved_(ustl::move(reserved)),
              start_address_(~PhysAddr(0)),
              end_address_(0),
              allocation_control_(AllocationControl::TopDown),
              allocation_lower_limit_(0),
              allocation_upper_limit_(ustl::NumericLimits<PhysAddr>::max())
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto trim(usize alignment) -> void {
            memories_.trim(alignment);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto add(PhysAddr base, usize size, RegionType type, NodeId nid = MAX_NODES) -> Status {
            if (base < start_address_) {
                start_address_ = base;
            }
            if (base + size > end_address_) {
                end_address_ = base + size;
            }
            return memories_.add(base, size, type, nid);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto remove(PhysAddr base, usize size) -> void {
            if (base == start_address_) {
                start_address_ = base + size;
            }
            if (base + size == end_address_) {
                end_address_ = base;
            }
            memories_.remove(base, size);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto protect(PhysAddr base, usize size) -> Status {
            return reserved_.add(base, size);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto allocate_bounded(usize size, AlignVal align, PhysAddr lower, PhysAddr upper, NodeId nid = MAX_NODES)
            -> PhysAddr {
            auto maybe_region = get_free_region(size, align, lower, upper, RegionType::Normal, nid);
            if (!maybe_region) {
                return 0;
            }
            protect(maybe_region->first, maybe_region->second - maybe_region->first);
            return maybe_region->first;
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        auto allocate(usize size, AlignVal align, PhysAddr lower, PhysAddr upper, NodeId nid = MAX_NODES) {  
            return reinterpret_cast<T *>(allocate_bounded(size, align, lower, upper, nid));  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto allocate(usize size, AlignVal align, NodeId nid = MAX_NODES) -> PhysAddr {
            return allocate_bounded(size, align, allocation_lower_limit_, allocation_upper_limit_, nid);
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        auto allocate(usize n, AlignVal align, NodeId nid = MAX_NODES) -> T * {  
            return reinterpret_cast<T *>(allocate(n * sizeof(T), align, nid));  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto deallocate(PhysAddr base, usize size) -> void {
            reserved_.remove(base, size);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_allocation_control(AllocationControl control) -> void {
            allocation_control_ = control;
        }

        CXX11_CONSTEXPR
        auto set_node(PhysAddr base, PhysAddr size, NodeId nid) -> void {
            auto maybe_range = memories_.isolate(base, size);
            if (!maybe_range) {
                return;
            }

            auto [start, end] = *maybe_range;
            for (auto i = start; i != end; ++i) {
                i->set_nid(nid);
            }
            memories_.try_merge(start, end);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto begin() -> IterMut {
            return IterMut(this,  memories_.begin(), reserved_.begin());
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto end() -> IterMut {
            return IterMut(this,  memories_.end(), reserved_.end());
        }

        struct IterationContext {
            FORCE_INLINE CXX11_CONSTEXPR
            IterationContext(PhysAddr start, PhysAddr end, RegionType type, NodeId nid = MAX_NODES)
                : memblock(0), start(start), end(end), type(type), nid(nid)
            {}

            FORCE_INLINE CXX11_CONSTEXPR
            IterationContext(RegionType type, NodeId nid = MAX_NODES)
                : memblock(0), start(0), end(ustl::NumericLimits<PhysAddr>::max()), type(type), nid(nid)
            {}

        private:
            friend MemBlock;
            MemBlock *memblock;
            PhysAddr start;
            PhysAddr end;
            RegionType type;
            NodeId nid;
            RegionList::IterMut  imem;
            RegionList::IterMut  ires;
        };
        auto iterate(IterationContext &context) -> ustl::Option<Region> {
            if (!context.memblock || context.memblock != this) {
                context.memblock = this;
                context.imem = memories_.begin();
                context.ires = reserved_.begin();
                context.start = ustl::algorithms::clamp(context.start, start_address_, end_address_);
                context.end = ustl::algorithms::clamp(context.end, start_address_, end_address_);
            }

            auto const iter_end = memories_.end();
            while (context.imem != iter_end) {
                if (context.nid != MAX_NODES) {
                    while (context.imem != iter_end) {
                        if (context.imem->nid() == context.nid) {
                            break;
                        }
                        ++context.imem;
                    }
                }

                NodeId nid = context.imem->nid();
                PhysAddr start, end;
                if (context.type == RegionType::Unused) {
                    auto region = lookup_next_free_region(memories_.begin(), memories_.end(),
                                                          reserved_.begin(), reserved_.end(),
                                                          context.imem, context.ires);
                    if (!region) {
                        return ustl::none();
                    }

                    start = ustl::get<0>(*region);
                    end = ustl::get<1>(*region);
                } else if (context.type == RegionType::Normal) {
                    if (context.imem == iter_end) {
                        return ustl::none();
                    }

                    start = context.imem->base;
                    end = context.imem->end();
                    ++context.imem;
                }

                start = ustl::algorithms::clamp(start, context.start, context.end);
                end = ustl::algorithms::clamp(end, context.start, context.end);

                if (start < end) {
                    return Region(start, end - start, context.type, nid);
                }
            };

            return ustl::none();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto start_address() -> PhysAddr {
            return start_address_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto end_address() -> PhysAddr {
            return end_address_;
        }

        // The following methods was marked public just for tests
        FORCE_INLINE CXX11_CONSTEXPR
        auto memories_list() -> RegionList & {
            return memories_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto reserved_list() -> RegionList & {
            return reserved_;
        }
    private:
        friend Collection<Region>;

        FORCE_INLINE
        auto get_free_region(usize size, AlignVal align, RegionType type, NodeId nid) {
            return get_free_region(size, align, start_address_, end_address_, type, nid);
        }

        auto get_free_region(usize size, AlignVal align, PhysAddr lower, PhysAddr upper, RegionType type, NodeId nid)
            -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>> {
            if (bool(allocation_control_ & AllocationControl::TopDown)) {
                return find_free_region_top_down(size, usize(align), lower, upper, type, nid);
            } else {
                return find_free_region_bottom_up(size, usize(align), lower, upper, type, nid);
            }
        }

        auto find_free_region_top_down(usize size, usize align, PhysAddr lower, PhysAddr upper, RegionType type, NodeId nid)
            -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>> {
            auto imem = memories_.end(), ires = reserved_.end();
            while (true) {
                auto region = lookup_prev_free_region(memories_.begin(), memories_.end(),
                                                      reserved_.begin(), reserved_.end(),
                                                      imem, ires);
                if (!region) {
                    return ustl::none();
                }

                NodeId const rnid = ustl::get<2>(*region);
                if (imem->type() != type) {
                    continue;
                } else if (rnid != nid && nid != MAX_NODES) {
                    continue;
                }
                PhysAddr start = ustl::algorithms::clamp(ustl::get<0>(*region), lower, upper);
                PhysAddr end = ustl::algorithms::clamp(ustl::get<1>(*region), lower, upper);

                PhysAddr aligned_start = ustl::mem::align_down(end - size, align);
                if (aligned_start < start || end - aligned_start < size) {
                    continue;
                }

                return ustl::make_pair(aligned_start, end);
            }
        }

        auto find_free_region_bottom_up(usize size, usize align, PhysAddr lower, PhysAddr upper, RegionType type, NodeId nid)
            -> ustl::Option<ustl::Pair<PhysAddr, PhysAddr>> {
            auto imem = memories_.begin(), ires = reserved_.begin();
            while (true) {
                auto region = lookup_next_free_region(memories_.begin(), memories_.end(),
                                                      reserved_.begin(), reserved_.end(),
                                                      imem, ires);
                if (!region) {
                    return ustl::none();
                }

                NodeId const rnid = ustl::get<2>(*region);
                if (imem->type() != type) {
                    continue;
                } else if (rnid != nid && nid != MAX_NODES) {
                    continue;
                }

                PhysAddr start = ustl::algorithms::clamp(ustl::get<0>(*region), lower, upper);
                PhysAddr end = ustl::algorithms::clamp(ustl::get<1>(*region), lower, upper);

                PhysAddr aligned_start = ustl::mem::align_down(end - size, align);
                if (aligned_start < start || end - aligned_start < size) {
                    continue;
                }

                return ustl::make_pair(aligned_start, end);
            }
        }

        RegionList memories_;
        RegionList reserved_;
        PhysAddr start_address_;
        PhysAddr end_address_;
        PhysAddr allocation_lower_limit_;
        PhysAddr allocation_upper_limit_;
        AllocationControl allocation_control_;
    }; // class MemBlock

    template <typename Region>
    struct RegionVector {
        typedef RegionVector    Self;
        typedef bootmem::Region const *  Iter;
        typedef bootmem::Region *        IterMut;
        typedef bootmem::MemBlock<RegionVector>   Holder;
        typedef ustl::iterator::RevIter<Iter>     RevIter;
        typedef ustl::iterator::RevIter<IterMut>  RevIterMut;

        RegionVector(Holder *holder = 0)
            : holder_(holder),
              count_(0),
              regions_(0),
              capacity_(0)
        {}

        RegionVector(Holder *holder, Region *raw, usize capacity)
            : holder_(holder),
              count_(0),
              regions_(raw),
              capacity_(capacity)
        {}

        auto erase(IterMut pos) -> IterMut {
            if (count_) {
                ustl::algorithms::copy(pos + 1, end(), pos);
                count_ -= 1;
            }

            return pos;
        }

        auto erase(IterMut first, IterMut last) -> void {
            if (last != end()) {
                ustl::algorithms::copy(last, end(), first);
            }
            count_ -= last - first;
        }

        template <typename... Args> 
        auto emplace(IterMut pos, Args &&...args) -> IterMut {
            if (count_ + 1 > capacity_) {
                grow(capacity_ << 1);
            }

            ustl::algorithms::copy(pos, end(), pos + 1);
            ustl::mem::construct_at(pos, ustl::forward<Args>(args)...);
            count_ += 1;
            return pos;
        }

        template <typename... Args> 
        auto emplace_back(Args &&...args) -> IterMut {
            return emplace(end(), ustl::forward<Args>(args)...);
        }

        auto reset(bootmem::Region *new_region, usize new_capacity) -> void {
            if (new_capacity < capacity_) {
                return;
            }
            ustl::algorithms::copy_n(regions_, count_, new_region);

            regions_ = new_region;
            capacity_ = new_capacity;
        }

        auto begin() const -> Iter {
            return regions_;
        }

        auto end() const -> Iter {
            return regions_ + count_;
        }

        auto begin() -> IterMut {
            return regions_;
        }

        auto end() -> IterMut {
            return regions_ + count_;
        }

        auto rbegin() const -> RevIter {
            return RevIter(end());
        }

        auto rend() const -> RevIter {
            return RevIter(begin());
        }

        auto size() const -> usize {
            return count_;
        }

        auto capacity() const -> usize {
            return capacity_;
        }

        auto back() -> Region & {
            return *(regions_ + count_ - 1);
        }

        auto reserve(usize additional) -> void {
            if (additional > capacity_ - count_) {
                DEBUG_ASSERT(grow(capacity_ + additional));
            }
        }

        auto grow(usize new_capacity) -> bool {
            auto base = reinterpret_cast<PhysAddr>(regions_);
            auto size = capacity_;
            if (auto new_base = holder_->get_free_region(new_capacity, AlignVal(alignof(usize)), RegionType::Normal, 0 )) {
                auto [start, end] = *new_base;
                reset(reinterpret_cast<Region *>(start), new_capacity);
                holder_->protect(start, end - start);
                // Give back the memory loaned
                holder_->deallocate(base, size);

                return true;
            }
            return false;
        }

        Holder *holder_;
        Region *regions_;
        usize capacity_;
        usize count_;
    };

} // namespace bootmem

#endif // #ifndef BOOTMEM_MEMBLOCK_HPP
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
#ifndef ARCH_PAGING_MAPPING_MAPPING_HPP
#define ARCH_PAGING_MAPPING_MAPPING_HPP 1

#include <arch/types.hpp>
#include <arch/paging/mmu_flags.hpp>
#include <arch/paging/arch_mmu_flags.hpp>
#include <ustl/option.hpp>
#include <ustl/result.hpp>
#include <ustl/util/pair.hpp>

namespace arch::paging {
    ///
    struct VirtAddrCursor {
        FORCE_INLINE CXX11_CONSTEXPR 
        VirtAddrCursor() = default;

        FORCE_INLINE CXX11_CONSTEXPR 
        VirtAddrCursor(VirtAddr virt, usize size)
            : addr_(virt),
              size_(size),
              consumed_(0) 
        {}

        FORCE_INLINE CXX11_CONSTEXPR 
        auto size() const -> usize {
            return size_;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto virt_addr() const -> VirtAddr {
            DEBUG_ASSERT(consumed_ < size_);
            return addr_ + consumed_;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto consume(usize page_size) -> void {
            consumed_ += page_size;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto remaining_size() const -> usize {  
            return size_ - consumed_; 
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto consumed_range() const -> VirtAddrCursor {
            return VirtAddrCursor(addr_, consumed_);
        }

        usize size_;
        usize consumed_;
        VirtAddr addr_;
    };

    struct PhysAddrCursor {
        FORCE_INLINE CXX11_CONSTEXPR 
        PhysAddrCursor() = default;

        FORCE_INLINE CXX11_CONSTEXPR 
        PhysAddrCursor(PhysAddr* phys, usize n, usize page_size)
            : page_size_(page_size),
              count_(n),
              addrs_(phys),
              consumed_(0) 
        {}

        FORCE_INLINE CXX11_CONSTEXPR 
        auto phys_addr() const -> PhysAddr {
            return (*addrs_) + consumed_;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto consume(usize page_size) -> void {
            DEBUG_ASSERT(consumed_ <= page_size_, "Consume");
            DEBUG_ASSERT(count_ > 0, "Consume");

            consumed_ += page_size;
            if (consumed_ == page_size_) {
                consumed_ = 0;
                count_ -= 1;
                addrs_ += 1;
            }
        }

        CXX11_CONSTEXPR
        auto remaining_size() const -> usize {
            if (!count_) {
                return 0;
            }
            return page_size_ - consumed_;
        }

        usize page_size_;
        usize consumed_;
        usize count_;
        PhysAddr* addrs_;
    };

    struct TravelContext {
        typedef TravelContext    Self;

        TravelContext() = default;

        TravelContext(VirtAddrCursor cursor, MmuFlags mmuf = {})
            : num_travelled_(0),
              virt_cursor_(cursor), 
              flags_(mmuflags_cast<ArchMmuFlags>(mmuf))
        {}

        TravelContext(VirtAddr va, usize nr_pages, usize page_size, MmuFlags mmuf = {})
            : Self(VirtAddrCursor(va, nr_pages * page_size), mmuf) 
        {}

        FORCE_INLINE CXX11_CONSTEXPR 
        auto consume(usize page_size) -> void {
            virt_cursor_.consume(page_size);
            num_travelled_ += 1;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize {  
            return virt_cursor_.size();  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mmuflags() const -> ArchMmuFlags {  
            return flags_;  
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto num_travelled() const -> usize {  
            return num_travelled_;  
        }

        FORCE_INLINE
        auto virt_cursor() -> VirtAddrCursor & {  
            return virt_cursor_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto virt_addr() const -> VirtAddr {  
            return virt_cursor_.virt_addr();  
        }

        FORCE_INLINE
        auto set_cursor(VirtAddrCursor cursor) -> Self & {
            virt_cursor_ = cursor;
            return *this;
        }

      protected:
        usize num_travelled_;
        ArchMmuFlags flags_;
        VirtAddrCursor virt_cursor_;
    };

    struct MapContext: public TravelContext {
        typedef MapContext      Self;
        typedef TravelContext   Base;

        MapContext() = default;

        FORCE_INLINE CXX11_CONSTEXPR
        MapContext(VirtAddr va, PhysAddr *pa, usize n, usize page_size, MmuFlags flags)
            : Base(va, n, page_size, flags), 
              phys_cursor_(pa, n, page_size)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_addr() const -> PhysAddr {  
            return phys_cursor_.phys_addr();  
        }

        FORCE_INLINE
        auto phys_cursor() -> PhysAddrCursor & {  
            return phys_cursor_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto num_mapped() const -> usize {
            return num_travelled_;
        }

        FORCE_INLINE
        auto consume(usize page_size) -> void {
            Base::consume(page_size);
            phys_cursor_.consume(page_size);
        }

        FORCE_INLINE
        auto skip(usize page_size) -> void {  
            virt_cursor_.consume(page_size);
        }

        FORCE_INLINE
        auto take(usize page_size) -> ustl::Pair<PhysAddr, VirtAddr> {
            auto const phys = phys_cursor_.phys_addr();
            auto const virt = virt_cursor_.virt_addr();
            this->consume(page_size);
            return ustl::make_pair(phys, virt);
        }

        FORCE_INLINE
        auto has_more() const -> bool {
            return phys_cursor_.count_ > 0;
        }

      protected:
        PhysAddrCursor phys_cursor_;
    };

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_MAPPING_MAPPING_HPP
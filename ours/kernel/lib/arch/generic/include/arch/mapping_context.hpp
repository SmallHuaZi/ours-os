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

#ifndef ARCH_MAPPING_CONTEXT_HPP
#define ARCH_MAPPING_CONTEXT_HPP 1

#include <arch/types.hpp>

#include <ustl/util/pair.hpp>

namespace arch {
    struct VirtAddrCursor
    {
        FORCE_INLINE CXX11_CONSTEXPR
        VirtAddrCursor() = default;

        FORCE_INLINE CXX11_CONSTEXPR
        VirtAddrCursor(VirtAddr virt, usize size)
            : addr_(virt),
              size_(size),
              consumed_(0)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize 
        {  return size_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto virt_addr() const -> VirtAddr
        {  return addr_ + consumed_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto consume(usize page_size) -> void
        {  consumed_ += page_size;  }

        usize size_;
        usize consumed_;
        VirtAddr addr_;
    };

    struct PhysAddrCursor
    {
        FORCE_INLINE CXX11_CONSTEXPR
        PhysAddrCursor() = default;

        FORCE_INLINE CXX11_CONSTEXPR
        PhysAddrCursor(PhysAddr *phys, usize n, usize page_size)
            : page_size_(page_size),
              count_(n),
              addrs_(phys),
              consumed_(0)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_addr() const -> PhysAddr
        {  return addrs_[count_] + consumed_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto consume(usize page_size) -> void
        {  
            DEBUG_ASSERT(consumed_ <= page_size, "");
            DEBUG_ASSERT(count_ > 0, "");

            consumed_ += page_size;
            if (consumed_ == page_size) {
                consumed_ = 0;
                count_ -= 1;
                addrs_ += 1;
            }
        }

        CXX11_CONSTEXPR
        auto remaining_size() const -> usize
        {  return page_size_ - consumed_;  }

        usize page_size_;
        usize consumed_;
        usize count_;
        PhysAddr *addrs_;
    };

    struct GenericMappingContext
    {
        GenericMappingContext() = default;

        GenericMappingContext(VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags, usize page_size)
            : flags_(flags),
              virt_cursor_(va, page_size * n),
              phys_cursor_(pa, n, page_size)
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto phys_addr() const -> PhysAddr
        {  return phys_cursor_.phys_addr();  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto virt_addr() const -> VirtAddr
        {  return virt_cursor_.virt_addr();  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto flags() const -> MmuFlags
        {  return flags_;  }

        FORCE_INLINE CXX11_CONSTEXPR
        auto size() const -> usize
        {  return virt_cursor_.size();  }

        CXX11_CONSTEXPR
        auto remaining_size() const -> usize
        {  return phys_cursor_.remaining_size();  }

        FORCE_INLINE
        auto virt_cursor() -> VirtAddrCursor &
        {  return virt_cursor_;  }

        FORCE_INLINE
        auto consume(usize page_size) -> void
        {
            phys_cursor_.consume(page_size);
            virt_cursor_.consume(page_size);
        }

        FORCE_INLINE
        auto skip(usize page_size) -> void
        {  virt_cursor_.consume(page_size);  }

        FORCE_INLINE
        auto take(usize page_size) -> ustl::Pair<PhysAddr, VirtAddr>
        {
            auto const phys = phys_cursor_.phys_addr();
            auto const virt = virt_cursor_.virt_addr();
            this->consume(page_size);
            return { phys, virt };
        }

        auto finish() -> void;

    private:
        MmuFlags flags_;
        PhysAddrCursor phys_cursor_;
        VirtAddrCursor virt_cursor_;
    };

} // namespace arch

#endif // #ifndef ARCH_MAPPING_CONTEXT_HPP
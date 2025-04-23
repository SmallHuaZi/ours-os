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
#ifndef OURS_MEM_VM_AREA_BASE_HPP
#define OURS_MEM_VM_AREA_BASE_HPP 1

#include <ours/types.hpp>
#include <ours/macro_abi.hpp>
#include <ours/mem/types.hpp>

#include <ustl/rc.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/collections/intrusive/set.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmaFlags: u32 {
        // For semantics, no any actual effects.
        None,

        // Only serve for VmMapping
        Write = BIT(1),
        Read  = BIT(2),
        Exec  = BIT(3),
        Share = BIT(4),

        // Serve for both VmMapping and VmArea
        MayWrite = BIT(5),
        MayRead  = BIT(6),
        MayExec  = BIT(7),
        MayShare = BIT(8),
        PermMask = MayWrite | MayRead | MayExec,

        // States 
        Active   = 0x10000,

        // Features.
        Anonymous = 0x40000,
        Mergeable = 0x80000,
    };
    USTL_ENABLE_ENUM_BITMASK(VmaFlags);

    class VmAreaBase: public ustl::RefCounter<VmAreaBase> {
        typedef VmAreaBase  Self;
        typedef ustl::RefCounter<VmAreaBase>    Base;
    public:
        FORCE_INLINE
        auto contains(VirtAddr va) const -> bool {
            return base_ <= va && va < base_ + size_;
        }

        FORCE_INLINE
        auto contains(VirtAddr va, usize len) const -> bool {
            return base_ <= va && va + len < base_ + size_;
        }

        FORCE_INLINE
        auto is_active() const -> bool {
            return !!(vmaf_ & VmaFlags::Active);
        }

        FORCE_INLINE
        auto is_mapping() const -> bool {
            CXX11_CONSTEXPR
            auto const mask = VmaFlags::Read | VmaFlags::Write | VmaFlags::Exec | VmaFlags::Share;
            return !!(vmaf_ & mask);
        }

        FORCE_INLINE
        auto base() const -> VirtAddr {
            return base_;
        }

        FORCE_INLINE
        auto size() const -> VirtAddr {
            return size_;
        }

        FORCE_INLINE
        auto start_vpn() const -> VirtAddr {
            return base_ >> PAGE_SHIFT;
        }

        FORCE_INLINE
        auto end_vpn() const -> VirtAddr {
            return ustl::mem::align_up(base_ + size_, PAGE_SIZE) >> PAGE_SHIFT;
        }

    protected:
        VmAreaBase(VirtAddr base, usize size, VmaFlags vmaf, char const *name)
            : base_(base), size_(size), vmaf_(vmaf), name_(name)
        {}

        /// Activate this VMA or Mapping, and modifies state of the to VmaFlags::Active.
        virtual auto activate() -> void = 0;

        GKTL_CANARY(VmAreaBase, canary_);
        char const *name_;
        VirtAddr  base_;
        VirtAddr  size_;
        VmaFlags  vmaf_;
        ustl::Rc<VmAspace> aspace_;
        ustl::collections::intrusive::SetMemberHook<> managed_hook_;

        /// Export to subclasses
    public:
        USTL_DECLARE_HOOK_OPTION(VmAreaBase, managed_hook_, ManagedOptions);
    };

    class VmAreaSet {
    public:
        FORCE_INLINE
        auto has_range(VirtAddr base, VirtAddr size) const -> bool {
            auto prev = vmaset_.upper_bound(base, AreaRangeComp());
            if (prev == vmaset_.end()) {
                return false;
            }
            auto next = prev--;
            if (next->base() < base + size && prev->base() + prev->size() < base) {
                return false;
            }

            return true;
        }

    private:
        struct AreaRangeComp {
            FORCE_INLINE CXX23_STATIC
            auto operator()(VmAreaBase const &x, VmAreaBase const &y) -> bool {
                return x.base() < y.base();
            }

            FORCE_INLINE CXX23_STATIC
            auto operator()(VirtAddr base, VmAreaBase const &x) -> bool {
                return base < x.base();
            }
        };
        USTL_DECLARE_MULTISET(VmAreaBase, VmaSet, VmAreaBase::ManagedOptions, 
            ustl::collections::intrusive::Compare<AreaRangeComp>);
        
        VmaSet vmaset_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_BASE_HPP
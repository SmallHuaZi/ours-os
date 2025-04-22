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

        // Features.
        Anonymous = 0x40000,
        Mergeable = 0x80000,

        // Categories
        Inactive = 0,
        Active   = 1,

        Normal   = 0x10000,
        Mapped   = 0x20000,
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
            return (base_ + size_) >> PAGE_SHIFT;
        }

        virtual auto activate() -> void = 0;
        
    protected:
        VmAreaBase(VirtAddr base, usize size, VmaFlags vmaf, char const *name)
            : base_(base), size_(size), vmaf_(vmaf), name_(name)
        {}

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
        auto has_range(VirtAddr base, VirtAddr size) const -> bool;

    private:
        struct AreaRangeComp {
            FORCE_INLINE CXX23_STATIC
            auto operator()(VmAreaBase const &x, VmAreaBase const &y) -> bool {
                return x.base() < y.base();
            }
        };
        USTL_DECLARE_MULTISET(VmAreaBase, VmaSet, VmAreaBase::ManagedOptions, 
            ustl::collections::intrusive::Compare<AreaRangeComp>);
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_BASE_HPP
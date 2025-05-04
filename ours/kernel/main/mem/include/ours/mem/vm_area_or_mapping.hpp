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
#ifndef OURS_MEM_VM_AREA_OR_MAPPING_HPP
#define OURS_MEM_VM_AREA_OR_MAPPING_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/vm_fault.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <ustl/rc.hpp>
#include <ustl/result.hpp>
#include <ustl/option.hpp>
#include <ustl/collections/intrusive/set.hpp>
#include <ustl/collections/intrusive/list.hpp>
#include <ustl/collections/intrusive/any_hook.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmMapOption {
        // For semenatics, no any effects.
        None,
        Commit = BIT(0),
        Pinned = BIT(1),
        Fixed = BIT(2),
        FixedNoReplace = BIT(3),
    };
    USTL_ENABLE_ENUM_BITMASK(VmMapOption);

    enum class VmaFlags: u32 {
        // For semantics, no any actual effects.
        None,

        // Only serve for VmMapping
        Write = BIT(1),
        Read  = BIT(2),
        Exec  = BIT(3),
        Share = BIT(4),
        PermMask = Write | Read | Exec | Share,

        // States 
        Active   = 0x10000,
        Pinned   = 0x20000,

        // Features.
        Anonymous = 0x40000,
        Mergeable = 0x80000,

        // Types
        Area = 0x0000'0000, // For semantics, no any actual effects.
        Mapping = 0x1000'0000,
    };
    USTL_ENABLE_ENUM_BITMASK(VmaFlags);

    FORCE_INLINE
    static auto extract_permissions(VmaFlags vmaflags) -> MmuFlags {
        MmuFlags mmuf{};
        if (!!(vmaflags & VmaFlags::Write)) {
            mmuf |= MmuFlags::Writable;
        }
        if (!!(vmaflags & VmaFlags::Read)) {
            mmuf |= MmuFlags::Readable;
        }
        if (!!(vmaflags & VmaFlags::Exec)) {
            mmuf |= MmuFlags::Executable;
        }

        return mmuf;
    }

    class VmAreaOrMapping: public ustl::RefCounter<VmAreaOrMapping> {
        typedef VmAreaOrMapping         Self;
        typedef ustl::RefCounter<Self>  Base;
      public:
        VmAreaOrMapping(VirtAddr base, usize size, VmaFlags vmaf, VmArea *, VmAspace *, char const *);

        FORCE_INLINE
        auto aspace() -> ustl::Rc<VmAspace> & {
            return aspace_;
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
        auto is_active() const -> bool {
            return !!(vmaf_ & VmaFlags::Active);
        }

        FORCE_INLINE
        auto lock() -> Mutex * {
            return aspace_->lock();
        }

        FORCE_INLINE
        auto check_range(VirtAddr base, usize size) const -> bool {
            return base >= base_ && size <= size_ && (base - base_) <= size_ - size;
        }

        FORCE_INLINE
        auto is_mapping() const -> bool {
            return !!(vmaf_ & VmaFlags::Mapping);
        }

        /// This for the collections to compare two VMA.
        FORCE_INLINE
        friend auto operator<(Self const &x, Self const &y) -> bool {
            return x.base_ < y.base_;
        }
      protected:
        /// Check whether a given `|mmuflags|` is valid. True if valid, otherwise invalid.
        auto validate_mmuflags(MmuFlags mmuf) const -> bool;

        /// Check whether a given `|vmaflags|` is valid. True if valid, otherwise invalid.
        FORCE_INLINE
        auto validate_vmaflags(VmaFlags vmaf) const -> bool {
            auto perm_allowed = (vmaf_ & VmaFlags::PermMask);
            if (((vmaf & VmaFlags::PermMask) | perm_allowed) != perm_allowed) {
                return false;
            }

            return true;
        }

        FORCE_INLINE
        auto mark_active() -> void {
            vmaf_ |= VmaFlags::Active;
        }

        virtual auto activate() -> Status; 

        /// This routine by default just remove itself.
        virtual auto destroy() -> Status; 

        friend class VmObject;
        friend class VmAspace;
        friend class VmaSet;
        friend class VmArea;

        GKTL_CANARY(VmAreaOrMapping, canary_);
        char const *name_;
        VirtAddr  base_;
        VirtAddr  size_;
        VmaFlags  vmaf_;
        VmArea   *parent_;
        ustl::Rc<VmAspace> aspace_;
        ustl::collections::intrusive::SetMemberHook<>   children_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, children_hook_, ManagedOptions);
    };
    USTL_DECLARE_MULTISET(VmAreaOrMapping, VmaSetBase, VmAreaOrMapping::ManagedOptions);

    class VmaSet: public VmaSetBase {
        typedef VmaSet       Self;
        typedef VmaSetBase   Base;

        using Base::insert;
        using Base::erase;
      public:
        class Enumerator;
        typedef Base::Element   Element;
        typedef Base::PtrMut    PtrMut;
        typedef Base::Ptr       Ptr;
        typedef Base::RefMut    RefMut;
        typedef Base::Ref       Ref;

        /// Determine that in this set if a region contains the range [base, base + size).
        ///
        /// Return true if contains, otherwise false.
        auto has_range(VirtAddr base, usize size) const -> bool;

        auto find_spot(usize size, AlignVal align, VirtAddr lower_bound, VirtAddr upper_bound)
            const -> ustl::Result<VirtAddr, Status>;
        
        // Now there are so many restrictions that we must manually manage those reference counters 
        // from instances of VmAom.
        
        FORCE_INLINE
        auto insert(Iter hint, RefMut value) -> IterMut {
            value.inc_strong_ref();
            return Base::insert(hint, value);
        }

        FORCE_INLINE
        auto insert(RefMut value) -> IterMut {
            value.inc_strong_ref();
            return Base::insert(value);
        }
 
        FORCE_INLINE
        auto erase(Iter pos) -> IterMut {
            pos.unconst()->dec_strong_ref();
            return Base::erase(pos);
        }       
 
        FORCE_INLINE
        auto erase(Iter first, Iter last) -> IterMut {
            return Base::erase_and_dispose(first, last, [] (PtrMut value) {
                value->dec_strong_ref();
            });
        }       

        /// Find the first existent VMA whose'end < addr.
        FORCE_INLINE
        auto lower_bound(VirtAddr addr) -> IterMut {
            return Base::lower_bound(addr, [] (Ref x, VirtAddr y) { return x.base_ < y; });
        }

        FORCE_INLINE
        auto lower_bound(VirtAddr addr) const -> Iter {
            return Base::lower_bound(addr, [] (Ref x, VirtAddr y) { return x.base_ < y; });
        }

        /// Find the first existent VMA whose'base > addr.
        FORCE_INLINE
        auto upper_bound(VirtAddr addr) -> IterMut {
            return Base::upper_bound(addr, [] (VirtAddr y, Ref x) { return y <= x.base_; });
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto upper_bound(VirtAddr addr) const -> Iter {
            return Base::upper_bound(addr, [] (VirtAddr y, Ref x) { return y <= x.base_; });
        }
    };

    class VmaSet::Enumerator {
      public:
        Enumerator(VmaSet &, VirtAddr min, VirtAddr max);
        Enumerator(VmArea &, VirtAddr min, VirtAddr max);
        ~Enumerator() = default;

      private:
        IterMut iter_;
        VmaSet *vmaset_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_OR_MAPPING_HPP
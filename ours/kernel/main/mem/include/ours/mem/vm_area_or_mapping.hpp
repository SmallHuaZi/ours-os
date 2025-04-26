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

    class VmArea;
    class VmMapping;

    class VmAreaOrMapping: public ustl::RefCounter<VmAreaOrMapping> {
        typedef VmAreaOrMapping         Self;
        typedef ustl::RefCounter<Self>  Base;
      public:
        VmAreaOrMapping(VirtAddr base, usize size, VmaFlags vmaf, VmArea *, VmAspace *, char const *);

        FORCE_INLINE
        auto aspace() -> ustl::Rc<VmAspace> {
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

        /// This for the collections to compare two VMA.
        FORCE_INLINE
        friend auto operator<(Self const &x, Self const &y) -> bool {
            return x.base_ < y.base_;
        }
      protected:
        /// Check whether a given `|mmflags|` is valid. True if valid, otherwise invalid.
        auto validate_mmuflags(MmuFlags mmuf) const -> bool;

        virtual auto activate() -> void = 0; 
        virtual auto destroy() -> void = 0; 

        friend class VmObject;
        friend class VmAspace;
        friend class VmArea;
        friend class VmMapping;
        friend class VmaSet;

        GKTL_CANARY(VmAreaOrMapping, canary_);
        char const *name_;
        VirtAddr  base_;
        VirtAddr  size_;
        VmaFlags  vmaf_;
        VmArea   *parent_;
        ustl::Rc<VmAspace> aspace_;
        ustl::collections::intrusive::SetMemberHook<> children_hook_;
      public:
        USTL_DECLARE_HOOK_OPTION(Self, children_hook_, ManagedOptions);
    };
    USTL_DECLARE_MULTISET(VmAreaOrMapping, VmaSetInner, VmAreaOrMapping::ManagedOptions);

    class VmaSet: public VmaSetInner {
        typedef VmaSet       Self;
        typedef VmaSetInner  Base;
      public:
        typedef VmAreaOrMapping Element;
        typedef Element *       PtrMut;
        typedef Element const * Ptr;
        typedef Element &       RefMut;
        typedef Element const & Ref;

        auto has_range(VirtAddr base, usize size) const -> bool;

        auto find_spot(usize size, AlignVal align, VirtAddr lower_bound, VirtAddr upper_bound)
            const -> ustl::Result<VirtAddr, Status>;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_AREA_OR_MAPPING_HPP
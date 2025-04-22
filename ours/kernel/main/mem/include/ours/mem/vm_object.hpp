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
#ifndef OURS_MEM_VM_OBJECT_HPP
#define OURS_MEM_VM_OBJECT_HPP

#include <ours/mem/types.hpp>
#include <ours/mem/vm_mapping.hpp>
#include <ours/status.hpp>

#include <ustl/option.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmoFLags {

    };
    USTL_ENABLE_ENUM_BITMASK(VmoFLags);

    class VmObject: public ustl::RefCounter<VmObject> {
        typedef VmObject     Self;
    public:
        enum class Type {
            Paged,
            Physical,
        };

        enum class CommitOptions {
            Pin = BIT(0),
            Write = BIT(1),
        };
        USTL_ENABLE_INNER_ENUM_BITMASK(CommitOptions);

        VmObject();

        virtual ~VmObject();

        /// 
        virtual auto acquire_pages(usize n) -> PhysAddr = 0;

        ///
        virtual auto release_pages(PhysAddr, usize) -> Status = 0;

        /// 
        virtual auto commit_range(PgOff pgoff, usize n, CommitOptions commit) -> Status = 0;

        ///
        virtual auto decommit(PgOff pgoff, usize n) -> Status = 0;

        ///
        virtual auto take_pages(gktl::Range<VirtAddr> range) -> Status = 0;

        ///
        virtual auto supply_pages(gktl::Range<VirtAddr> range) -> Status = 0;

        FORCE_INLINE
        auto commit_range_pinned(usize offset, usize len, bool write) -> Status {
            auto options = CommitOptions::Pin;
            if (write) {
                options |= CommitOptions::Write;
            }
            return commit_range(offset, len, options);
        }

        USTL_NO_MOVEABLE_AND_COPYABLE(VmObject);
    protected:
        GKTL_CANARY(VmObject, canary_);
        Type const type_;
        VmoFLags flags_;
        VmMappingList mappings_;
    
        ustl::collections::intrusive::ListMemberHook<> children_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, children_hook_, ChildrenOptions);
        ustl::collections::intrusive::List<Self, ChildrenOptions>  children_;
    };

} // namespace ours::mem 

#endif // #ifndef OURS_MEM_PM_OBJECT_HPP 
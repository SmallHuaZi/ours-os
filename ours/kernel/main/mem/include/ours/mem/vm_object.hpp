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
#include <ours/mem/vm_page.hpp>
#include <ours/status.hpp>

#include <ustl/option.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/range.hpp>
#include <gktl/canary.hpp>

namespace ours::mem {
    enum class VmoFLags {
        /// Can not be swaped out.
        Pinned = BIT(0),

        /// Never pre-commit pages, only when the page access fault occurs.
        Lazy = BIT(1),
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
            /// No any effect, just for semantics.
            None,

            /// The pages mapped will not be swaped out.
            Pin = BIT(0),

            /// Map imediately but wait a page access fault come.
            Write = BIT(1),
        };
        USTL_ENABLE_INNER_ENUM_BITMASK(CommitOptions);

        /// Actively request pages for fulfilling range [pgoff, pgoff + n).
        virtual auto commit_range(PgOff pgoff, usize n, CommitOptions commit = CommitOptions::None) -> Status = 0;

        ///
        virtual auto decommit(PgOff pgoff, usize n) -> Status = 0;

        /// Remove all pages in range [ `|pgoff|`, `|pgoff + n|`) to `|page_list|`.
        virtual auto take_pages(PgOff pgoff, usize n, VmPageList *page_list) -> Status {
            return Status::Unsupported;
        } 

        /// Supplies this VMO a group of page for range [pgoff, pgoff + n).
        /// Typically it should be invoked by PageProvider.
        virtual auto supply_pages(PgOff pgoff, usize n, VmPageList *page_list) -> Status {
            return Status::Unsupported;
        } 

        ///
        virtual auto read(void *out, PgOff pgoff, usize n) -> Status {
            return Status::Unsupported;
        }

        ///
        virtual auto write(void *out, PgOff pgoff, usize n) -> Status {
            return Status::Unsupported;
        }

        FORCE_INLINE
        auto commit_range_pinned(usize offset, usize len, bool write) -> Status {
            auto options = CommitOptions::Pin;
            if (write) {
                options |= CommitOptions::Write;
            }
            return commit_range(offset, len, options);
        }

        FORCE_INLINE
        auto type() const -> Type {
            return type_;
        }

        USTL_NO_MOVEABLE_AND_COPYABLE(VmObject);
    protected:
        VmObject(Type type, VmoFLags vmoflags);

        virtual ~VmObject() = default;

        GKTL_CANARY(VmObject, canary_);
        Type const type_;
        VmoFLags vmof_;
        VmMappingList mappings_;
    
        ustl::collections::intrusive::ListMemberHook<> children_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, children_hook_, ChildrenOptions);
        ustl::collections::intrusive::List<Self, ChildrenOptions>  children_;
    };

    template <typename VmObjectDerived>
    FORCE_INLINE CXX11_CONSTEXPR
    auto downcast(ustl::Rc<VmObject> vmo) -> ustl::Rc<VmObjectDerived> {
        if CXX17_CONSTEXPR (ustl::traits::IsSameV<VmObjectDerived, VmObjectPaged>) {
            if (vmo->type() == VmObject::Type::Paged) {
                return ustl::make_rc<VmObjectDerived>(vmo);
            }
        }
        return nullptr;
    }

    template <typename VmObjectDerived>
    FORCE_INLINE CXX11_CONSTEXPR
    auto downcast(VmObject *vmo) -> VmObjectDerived * {
        if CXX17_CONSTEXPR (ustl::traits::IsSameV<VmObjectDerived, VmObjectPaged>) {
            if (vmo->type() == VmObject::Type::Paged) {
                return static_cast<VmObjectDerived *>(vmo);
            }
        }
        return nullptr;
    }

} // namespace ours::mem 

#endif // #ifndef OURS_MEM_PM_OBJECT_HPP 
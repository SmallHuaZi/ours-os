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
#ifndef OURS_MEM_VM_OBJECT_PAGED_HPP
#define OURS_MEM_VM_OBJECT_PAGED_HPP 1

#include <ours/mem/vm_object.hpp>
#include <ours/mem/vm_cow_pages.hpp>
#include <ours/mutex.hpp>

#include <logz4/log.hpp>
#include <ustl/result.hpp>

namespace ours::mem {
    class VmObjectPaged: public VmObject {
        typedef VmObject        Base;
        typedef VmObjectPaged   Self;
    public:
        static auto create(Gaf gaf, usize nr_pages, VmoFLags vmof, ustl::Rc<VmObjectPaged> *out) -> Status;

        static auto create_contiguous(Gaf gaf, usize nr_pages, VmoFLags vmof, ustl::Rc<VmObjectPaged> *out) -> Status;

        /// 
        virtual auto commit_range(PgOff offset, usize n, CommitOptions option) -> Status override;

        ///
        virtual auto decommit(PgOff pgoff, usize n) -> Status override;

        ///
        virtual auto take_pages(PgOff pgoff, usize n, VmPageList *pagelist) -> Status override;

        ///
        virtual auto supply_pages(PgOff pgoff, usize n, VmPageList *pagelist) -> Status override;

        ///
        virtual auto read(void *out, PgOff pgoff, usize size) -> Status override;

        ///
        virtual auto write(void *out, PgOff pgoff, usize size) -> Status override;

        FORCE_INLINE
        auto make_cursor(PgOff pgoff, usize n) -> ustl::Result<VmCowPages::Cursor, Status> {
            return cow_pages_->make_cursor(pgoff, n);
        }

        /// Priviate on logic, please go to use the facotry member like VmObjectPaged::create*.
        VmObjectPaged(VmoFLags vmof, ustl::Rc<VmCowPages>);
        virtual ~VmObjectPaged() = default;
    private:
        auto commit_range_internal(PgOff offset, usize n, CommitOptions option) -> Status;

        VmMappingList mappings_;
        ustl::Rc<VmCowPages> cow_pages_;
        Mutex mutex_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_OBJECT_PAGED_HPP
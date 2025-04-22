// ours/mem OURS/MEM_VM_OBJECT_PAGED_HPP
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

namespace ours::mem {
    class VmObjectPaged: public VmObject {
    public:
        static auto create() -> ustl::Rc<VmObjectPaged>;
        static auto create_contiguous() -> ustl::Rc<VmObjectPaged>;

        /// 
        virtual auto acquire_pages(usize n) -> PhysAddr override;

        ///
        virtual auto release_pages(PhysAddr, usize) -> Status override;

        ///
        virtual auto commit_range(PgOff offset, usize n, CommitOptions option) -> Status override;

        ///
        virtual auto decommit(usize offset, usize len) -> Status override;

        ///
        virtual auto take_pages(gktl::Range<VirtAddr> range) -> Status override;

        ///
        virtual auto supply_pages(gktl::Range<VirtAddr> range) -> Status override;
    private:
        auto commit_range_internal(PgOff offset, usize n, CommitOptions option) -> Status;

        VmCowPages cow_pages_;
        Mutex mutex_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_OBJECT_PAGED_HPP
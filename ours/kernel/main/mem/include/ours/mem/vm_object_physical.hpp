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
#ifndef OURS_MEM_VM_OBJECT_PHYSICAL_HPP
#define OURS_MEM_VM_OBJECT_PHYSICAL_HPP 1

#include <ours/mem/vm_object.hpp>

namespace ours::mem {
    class VmObjectPhysical: public VmObject {
        typedef VmObjectPhysical    Self;
        typedef VmObject            Base;
      public:
        static auto create(PhysAddr pa, usize size, VmoFLags vmoflags, ustl::Rc<Self> *out) -> Status;

        /// 
        virtual auto commit_range(VirtAddr offset, usize size, CommitOptions option) -> Status override {
            if (offset > size_ || size > size_ || offset > size_ - size) {
                return Status::OutOfRange;
            }
            return Status::Ok;
        }

        auto lookup_range(usize offset, usize size, PhysAddr *pa) -> Status {
            if (offset > size_ || size > size_ || offset > size_ - size) {
                return Status::OutOfRange;
            }

            *pa = phys_base_ + offset;
            return Status::Ok;
        }

      private:
        VmObjectPhysical(PhysAddr pa, usize size, VmoFLags vmoflags);

        PhysAddr phys_base_;
        usize size_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_OBJECT_PHYSICAL_HPP
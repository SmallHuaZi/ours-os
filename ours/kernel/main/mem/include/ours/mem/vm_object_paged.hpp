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

namespace ours::mem {
    class VmObjectPaged
        : public VmObject
    {
    public:
        static auto create() -> ustl::Rc<VmObjectPaged>;
        static auto create_contiguous() -> ustl::Rc<VmObjectPaged>;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_OBJECT_PAGED_HPP
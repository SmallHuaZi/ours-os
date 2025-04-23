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
#ifndef OURS_MEM_DETAILS_VM_OBJECT_CACHES_HPP
#define OURS_MEM_DETAILS_VM_OBJECT_CACHES_HPP 1

#include <ours/mem/object-cache.hpp>
#include <ustl/array.hpp>

namespace ours::mem {
    enum class CacheType {
        VmObjectPaged,
        VmAspace,
        VmArea,
        VmMapping,
        VmCowPages,
        MaxNumCaches,
    };

    extern ustl::Array<ustl::Rc<ObjectCache>, usize(CacheType::MaxNumCaches)> g_vm_caches;

    template <CacheType Type>
    FORCE_INLINE
    static auto get_vm_cache() -> ObjectCache * {
        return g_vm_caches[usize(Type)].as_ptr_mut();
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_DETAILS_VM_OBJECT_CACHES_HPP
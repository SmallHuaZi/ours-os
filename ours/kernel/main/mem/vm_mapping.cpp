#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/object-cache.hpp>

namespace ours::mem {
    static VmMappingHandler s_normal_vma_handler;
    static VmMappingHandler s_mapping_vma_handler;

    static ustl::Rc<ObjectCache> s_vm_mapping_cache;

    VmMapping::VmMapping(VirtAddr base, usize size, VmArea *vma, VmaFlags vmaf, 
                         PgOff vmo_off, ustl::Rc<VmObject> vmo, MmuFlags mmuf, 
                         const char *name)
        : Base(base, size, vmaf, name),
          vmo_pgoff_(vmo_off), vmo_(ustl::move(vmo)), mmuf_(mmuf)
    {}

    auto VmMapping::create(VirtAddr base, usize size, VmArea *vma, VmaFlags vmaf, 
                           PgOff vmo_off, ustl::Rc<VmObject> vmo, MmuFlags mmuf, 
                           const char *name)
        -> ustl::Result<ustl::Rc<Self>, Status> {
        if (!vmo) {
            return ustl::err(Status::InvalidArguments);
        }

        auto mapping = s_vm_mapping_cache->allocate<VmMapping>(base, size, vma, vmaf, vmo_off, vmo, mmuf, name);
        if (!mapping) {
            return ustl::err(Status::OutOfMem); 
        }

        return ustl::ok(ustl::make_rc<Self>(mapping));
    }

    auto VmMapping::map_range(PgOff pgoff, usize size, bool commit, MapControl control) -> Status {

    }


    auto VmMapping::unmap_range(PgOff pgoff, usize size, UnMapControl control) -> Status {

    }

} // namespace ours::mem
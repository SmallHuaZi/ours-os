#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/vm_object.hpp>
#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/mem/page_request.hpp>

#include <ustl/mem/align.hpp>
#include <gktl/init_hook.hpp>

namespace ours::mem {
    /// This helper class was used to batch mapping requests.
    template <usize MaxNumPages>
    class MappingCoalescer {
    public:
        MappingCoalescer(VmMapping *mapping, VirtAddr va, MmuFlags mmuf)
            : mapping_(mapping),
              va_(va),
              mmuf_(mmuf)
        {}

        auto append(PhysAddr pa) -> Status;
        auto commit() -> Status;

    private:
        VmMapping *mapping_;
        VirtAddr va_;
        PhysAddr pa_[MaxNumPages];
        usize nr_pages_;
        MmuFlags mmuf_;
        MapControl map_ctrl_;
    };

    template <usize MaxNumPages>
    FORCE_INLINE
    auto MappingCoalescer<MaxNumPages>::append(PhysAddr pa) -> Status {
        pa_[nr_pages_++] = pa;
        if (nr_pages_ == MaxNumPages) {
            return commit();
        }
        return Status::Ok;
    }

    template <usize MaxNumPages>
    FORCE_INLINE
    auto MappingCoalescer<MaxNumPages>::commit() -> Status {
        if (!nr_pages_) {
            return Status::Ok;
        }

        auto status = mapping_->aspace()
                              ->arch_aspace()
                              .map_bulk(va_, pa_, nr_pages_, mmuf_, MapControl::TryLargePage);
        if (Status::Ok != status) {
            log::error("Failed to map {} pages at {}", nr_pages_, va_);
            return status;
        }

        va_ += nr_pages_ * PAGE_SIZE;
        nr_pages_ = 0;
    }

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

    auto VmMapping::activate() -> void {}

    FORCE_INLINE
    auto VmMapping::map_range_paged(usize offset, usize size, bool commit, MapControl control, VmObjectPaged *vmo) -> Status {
        CXX11_CONSTEXPR
        static auto const kMaxBatchPages = 32;
        MappingCoalescer<kMaxBatchPages> coalescer(this, base() + offset, mmuf_);

        PageRequest page_request;
    }

    /// Do check arguments and dispatch the request to correct sub-routine for different VmObjects.
    auto VmMapping::map_range(usize offset, usize size, bool commit, MapControl control) -> Status {
        if (!size) {
            return Status::InvalidArguments;
        }

        size = ustl::mem::align_up(size, PAGE_SIZE);
        if (!contains(base_ + offset, size)) {
            return Status::InvalidArguments;
        }

        if (!is_active()) {
            return Status::BadState;
        }

        if (auto vmo = downcast<VmObjectPaged>(vmo_.as_ptr_mut())) {
            return map_range_paged(offset, size, commit, control, vmo);
        }

        return Status::Ok;
    }


    auto VmMapping::unmap_range(PgOff pgoff, usize size, UnMapControl control) -> Status {

    }

    INIT_CODE
    static auto init_vm_mapping_cache() -> void {
        s_vm_mapping_cache = ObjectCache::create<VmMapping>("vm-mapping-cache", OcFlags::Folio);
        if (!s_vm_mapping_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("VmMappingCache has been created");
    }
    GKTL_INIT_HOOK(VmMappingCacheInit, init_vm_mapping_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::mem
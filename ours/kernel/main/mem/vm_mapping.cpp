#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/mem/page_request.hpp>

#include <ustl/mem/align.hpp>
#include <gktl/init_hook.hpp>
#include <ktl/new.hpp>

namespace ours::mem {
    /// This helper class was used to batch mapping requests.
    template <usize MaxNumPages>
    class MappingCoalescer {
    public:
        MappingCoalescer(VmMapping *mapping, PgOff pgoff, MmuFlags mmuf, MapControl ctrl)
            : mapping_(mapping),
              va_(mapping->base() + (pgoff << PAGE_SHIFT)),
              mmuf_(mmuf),
              map_ctrl_(ctrl)
        {}

        auto append(VmPage *page) -> Status;
        auto commit() -> Status;

    private:
        VmMapping *mapping_;
        VirtAddr va_;
        PhysAddr pa_[MaxNumPages];
        usize nr_pages_;
        usize total_mapped_;
        MmuFlags mmuf_;
        MapControl map_ctrl_;
    };

    template <usize MaxNumPages>
    FORCE_INLINE
    auto MappingCoalescer<MaxNumPages>::append(VmPage *page) -> Status {
        pa_[nr_pages_++] = frame_to_phys(page);
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

        usize nr_mapped = 0;
        auto status = mapping_->aspace()
                              ->arch_aspace()
                              .map_bulk(va_, pa_, nr_pages_, mmuf_, MapControl::TryLargePage, &nr_mapped);
        if (Status::Ok != status) {
            log::error("Failed to map {} pages at {}", nr_pages_, va_);
        }

        va_ += nr_pages_ * PAGE_SIZE;
        nr_pages_ = 0;
        total_mapped_ += nr_mapped;
        return Status::Ok;
    }

    static ObjectCache *s_vm_mapping_cache;

    VmMapping::VmMapping(VirtAddr base, usize size, VmArea *vma,
                         PgOff vmo_off, ustl::Rc<VmObject> vmo, MmuFlags mmuf, 
                         const char *name)
        : base_(base), size_(size), vma_(vma), name_(name),
          vmo_pgoff_(vmo_off), vmo_(ustl::move(vmo)), mmuf_(mmuf)
    {}

    auto VmMapping::create(VirtAddr base, usize size, VmArea *vma, 
                           PgOff vmo_off, ustl::Rc<VmObject> vmo, MmuFlags mmuf, 
                           const char *name, ustl::Rc<Self> *out) -> Status {
        if (!vmo) {
            return Status::InvalidArguments;
        }

        auto mapping = new (*s_vm_mapping_cache, kGafKernel) VmMapping(base, size, vma, vmo_off, vmo, mmuf, name);
        if (!mapping) {
            return Status::OutOfMem; 
        }
        *out = ustl::make_rc<Self>(mapping);

        return Status::Ok;
    }

    FORCE_INLINE
    auto VmMapping::check_sburange(PgOff pgoff, usize nr_pages) const -> bool {
        return (pgoff + nr_pages) < (size_ >> PAGE_SHIFT);
    }

    auto VmMapping::activate() -> void {
        vma_->mapping_ = this;
    }

    FORCE_INLINE
    auto VmMapping::map_paged(PgOff pgoff, usize nr_pages, bool commit, MapControl control, VmObjectPaged *vmo) -> Status {
        CXX11_CONSTEXPR
        static auto const kMaxBatchPages = 32;
        MappingCoalescer<kMaxBatchPages> coalescer(this, pgoff, mmuf_, control);

        auto cursor = vmo->make_cursor(pgoff, nr_pages);
        if (!cursor) {
            return cursor.unwrap_err();
        }

        PageRequest page_request;
        for (auto i = 0; i < nr_pages; ++i) {
            auto result = cursor->require_owned_page(1, &page_request);
            if (!result) {
                return result.unwrap_err();
            }

            coalescer.append(*result);
        }
        // Commit those uncovered units in loop above.
        coalescer.commit();

        return Status::Ok;
    }

    /// Do check arguments and dispatch the request to correct sub-routine for different VmObjects.
    auto VmMapping::map(PgOff pgoff, usize nr_pages, bool commit, MapControl control) -> Status {
        canary_.verify();

        if (!nr_pages) {
            return Status::InvalidArguments;
        }

        if (!check_sburange(pgoff, nr_pages)) {
            return Status::InvalidArguments;
        }

        if (auto vmo = downcast<VmObjectPaged>(vmo_.as_ptr_mut())) {
            return map_paged(pgoff, nr_pages, commit, control, vmo);
        } else {
            DEBUG_ASSERT(false, "Unreachable");
        }

        return Status::Ok;
    }

    auto VmMapping::protect(PgOff, usize nr_pages, usize size, MmuFlags mmuf) -> Status {
        return Status::Unimplemented;
    }

    auto VmMapping::unmap(PgOff pgoff, usize size, UnmapControl control) -> Status {
        return Status::Unimplemented;
    }

    auto VmMapping::fault(VmFault *vmf) -> void {
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
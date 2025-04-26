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
     MappingRegionSet::MappingRegionSet(usize size, MmuFlags mmuf)
        : regions_(kNumInitialRegions) {
        regions_.emplace_back(size, mmuf);
     }

     auto MappingRegionSet::update(VirtAddr base, usize size, MmuFlags mmuf, 
                                   VirtAddr lower_bound, VirtAddr upper_bound) -> Enumerator {
        using ustl::algorithms::min;

        usize end = base + size, rbase = 0;
        for (auto i = 0; i < regions_.size(); rbase = regions_[i].end, ++i) {
            auto rend = regions_[i].end;
            if (rbase >= end) {
                break;
            }
            if (rend <= base) {
                continue;
            }

            if (mmuf == regions_[i].mmuf) {
                // Have a same MMU flags, no any need to split it.
                rbase = min(end, rend);
                continue;
            }

            // The code bottom will handle the intersection
            if (rbase < base) {
                // Rectifies this region to [rbase, base)
                regions_[i].end = base;
                base = min(end, rend);
                regions_.emplace(regions_.begin() + i + 1, base, mmuf);
            } else if (rend > end) {
                regions_.emplace(regions_.begin() + i, end, mmuf);
                regions_[i + 1].end = rend;
            } else {
                regions_[i].mmuf = mmuf;
            }
            rbase = regions_[i].end;
        }
     }

    /// This helper class was used to batch mapping requests.
    template <usize MaxNumPages>
    class MappingCoalescer {
    public:
        MappingCoalescer(VmMapping *mapping, VirtAddr base, MmuFlags mmuf, MapControl ctrl)
            : mapping_(mapping),
              va_(base),
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

    VmMapping::VmMapping(VmArea *parent, VirtAddr base, usize size, VmaFlags vmaf,
                         ustl::Rc<VmObject> vmo, usize vmo_off, MmuFlags mmuf, 
                         const char *name)
        : Base(base, size, vmaf, parent, parent->aspace().as_ptr_mut(), name),
          vmo_off_(vmo_off), vmo_(ustl::move(vmo)), regions_(size, mmuf)
    {}

    auto VmMapping::create(VmArea *parent, VirtAddr base, usize size, VmaFlags vmaf,
                           ustl::Rc<VmObject> vmo, usize vmo_off, MmuFlags mmuf, 
                           char const *name, ustl::Rc<Self> *out) -> Status {
        if (!vmo) {
            return Status::InvalidArguments;
        }

        auto mapping = new (*s_vm_mapping_cache, kGafKernel) VmMapping(
            parent, base, size, vmaf, vmo, vmo_off, mmuf, name);

        if (!mapping) {
            return Status::OutOfMem; 
        }
        *out = ustl::make_rc<Self>(mapping);

        return Status::Ok;
    }

    FORCE_INLINE
    auto VmMapping::check_sburange(VirtAddr base, usize size) const -> bool {
        return base >= base_ && (base + size) < (base_ + size_);
    }

    auto VmMapping::activate() -> void {
    }

    FORCE_INLINE
    auto VmMapping::map_paged(VirtAddr base, usize size, bool commit, MapControl control, VmObjectPaged *vmo) -> Status {
        DEBUG_ASSERT(ustl::mem::is_aligned(base, PAGE_SIZE));
        DEBUG_ASSERT(ustl::mem::is_aligned(size, PAGE_SIZE));

        CXX11_CONSTEXPR
        static auto const kMaxBatchPages = 32;
        MappingCoalescer<kMaxBatchPages> coalescer(this, base, mmuf_, control);

        auto cursor = vmo->make_cursor(base, size);
        if (!cursor) {
            return cursor.unwrap_err();
        }

        PageRequest page_request;
        for (auto i = 0; i < size; i += PAGE_SIZE) {
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
    auto VmMapping::map(VirtAddr offset, usize size, bool commit, MapControl control) -> Status {
        canary_.verify();

        size = ustl::mem::align_up(size, PAGE_SIZE);
        if (!size) {
            return Status::InvalidArguments;
        }

        VirtAddr base = ustl::mem::align_down(base_ + offset, PAGE_SIZE);
        if (!check_sburange(base, size)) {
            return Status::InvalidArguments;
        }

        if (auto vmo = downcast<VmObjectPaged>(vmo_.as_ptr_mut())) {
            return map_paged(base, size, commit, control, vmo);
        } else {
            DEBUG_ASSERT(false, "Unreachable");
        }

        return Status::Ok;
    }

    auto VmMapping::protect(usize offset, usize size, MmuFlags mmuf) -> Status {
        canary_.verify();

        if (!is_active()) {
            return Status::BadState;
        }

        size = ustl::mem::align_up(size, PAGE_SIZE);
        if (!size) {
            return Status::InvalidArguments;
        }

        ustl::sync::LockGuard guard(*lock());
        VirtAddr base = ustl::mem::align_down(base_ + offset, PAGE_SIZE);
        if (!check_sburange(base, size)) {
            return Status::InvalidArguments;
        }

        if (!validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        auto &arch_aspace = aspace_->arch_aspace();
        auto enumerator = regions_.update(offset, size, mmuf, 0, size);
        enumerator.shift(base_);
        while (auto region = enumerator.next()) {
            auto [base, size, mmuf] = *region;

            if (!(mmuf & MmuFlags::PermMask)) {
                // Unmap those regions which has no any permission.
                arch_aspace.unmap(base, size >> PAGE_SHIFT, UnmapControl::None, 0);
                continue;
            }

            arch_aspace.protect(base, size >> PAGE_SHIFT, mmuf);
        }

        return Status::Ok;
    }

    auto VmMapping::unmap(usize offset, usize size, UnmapControl control) -> Status {
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
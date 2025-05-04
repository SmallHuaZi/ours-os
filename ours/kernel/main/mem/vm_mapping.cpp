#include <ours/mem/vm_mapping.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/vm_object_physical.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/mem/page_request.hpp>

#include <ustl/mem/align.hpp>
#include <ustl/algorithms/search.hpp>
#include <ustl/collections/static-vec.hpp>
#include <ustl/iterator/function.hpp>
#include <gktl/init_hook.hpp>
#include <ktl/new.hpp>

#include <iterator>

namespace ours::mem {
    static ObjectCache *s_vm_mapping_cache;
    static ObjectCache *s_vm_mapping_region_cache;

    template <typename... Args>
    FORCE_INLINE
    auto MappingRegionSet::alloc_region(Args &&...args) -> Region * {
        return new (*s_vm_mapping_region_cache, kGafKernel) Region(args...);
    }

    FORCE_INLINE
    auto MappingRegionSet::free_region(Region *region) -> void {
        s_vm_mapping_region_cache->deallocate(region);
    }

    FORCE_INLINE
    auto MappingRegionSet::init(VirtAddr end, MmuFlags mmuf) -> Status {
        auto first_region = alloc_region(end, mmuf);
        if (!first_region) {
            return Status::OutOfMem;
        }

        regions_.insert(*first_region);
        return Status::Ok;
    }

    auto MappingRegionSet::make_enumerator(VirtAddr base, usize size) -> Enumerator {
        using namespace ustl::algorithms;
        auto first = lower_bound(regions_.begin(), regions_.end(), base, [] (auto const &x, auto y) {
            return x.end < y;
        });

        auto last = upper_bound(regions_.begin(), regions_.end(), base, [] (auto const &x, auto y) {
            return x <= y.end;
        });

        if (last != regions_.end()) {
            ++last;
        }

        return Enumerator(base, first, last, base, base + size);
    }

    auto MappingRegionSet::update(VirtAddr base, usize size, MmuFlags mmuf,
                                  VirtAddr lower_limit, VirtAddr upper_limit) -> Status {
        using namespace ustl::algorithms;

        // First, find the range that need to be handled.
        VirtAddr end = base + size;
        auto first = lower_bound(regions_.begin(), regions_.end(), base, [] (auto const &x, auto y) {
            return x.end < y;
        });
        auto last = upper_bound(first, regions_.end(), end, [] (auto x, auto const &y) {
            return x <= y.end;
        });

        auto const same_region = first == last;
        auto const first_mmuf = first->mmuf;
        auto const last_mmuf = last->mmuf;

        i32 new_regions_needed = 0;
        VirtAddr covered_end = last->end;
        if (first_mmuf == mmuf) {
            if (last_mmuf != mmuf) {
                // `first` and `last` do not represent the same region, 
                // so need we to reserve `first` and `last` simoutaneously.
                first->end = max(first->end, base + size);
                ++first;
            }

            // Otherwise it is enough to just reserve `last`.
        } else {
            // The ranges [base, end) and [first->end, last->end) may be mergeable.
            // It mainly occurs at the following cases:
            //      1. The former covers entire the latter.
            //      2. The former ownes the same MMU flags with the latter.
            // However which case happened, we all insert the range [xx, end), so
            // code after can just determines the cases `new_regions_needed != 0` and
            // `new_regions_needed > 1`.
            new_regions_needed = (base + size != covered_end && last_mmuf != mmuf) + same_region;

            // This case requires us to forcely reserve `first`.
            first->end = base;
            ++first;
        }

        // Before to do split, we should check if there are any reclaimable regions to avoid explicit 
        // memory allocation latter as far as possible, because it is possible failed to allocate 
        // a new region and that will lead us into an unrecoverable status.
        i32 nr_reclaimable;
        if (same_region) {
            nr_reclaimable = 0;
        } else {
            nr_reclaimable = std::distance(first, last);
        }

        // Reclaimable regions are insufficient to support this split operation.
        // We have to allocate remaining regions.
        ustl::collections::StaticVec<Region *, 2> reuse;
        if (nr_reclaimable < new_regions_needed) {
            auto const n = new_regions_needed - nr_reclaimable;
            while (reuse.size() != n) {
                auto new_region = alloc_region();
                if (!new_region) {
                    return Status::OutOfMem;
                }
                reuse.push_back(new_region);
            }
        }

        // There is no any memory allocation request, so we can start to reclaim those unused regions.
        regions_.erase_and_dispose(first, last, [&, new_regions_needed] (Region *region) { 
            if (reuse.size() < new_regions_needed) {
                reuse.push_back(region);
            } else {
                free_region(region);
            }
        });

        // Now we have done all memory allocation.
        if (new_regions_needed) {
            if (new_regions_needed > 1) { // The case `new_regions_needed == 2`
                new (reuse[1]) Region(base + size, mmuf);
                last = regions_.insert(last, *reuse[1]);
            }

            new (reuse[0]) Region(covered_end, last_mmuf);
            regions_.insert(last, *reuse[0]);
        }

        return Status::Ok;
    }

    /// This helper class was used to batch mapping requests.
    template <usize MaxNumPages>
    class MappingCoalescer {
    public:
        MappingCoalescer(VmMapping *mapping, VirtAddr base, MmuFlags mmuf, MapControl ctrl)
            : mapping_(mapping),
              va_(base),
              mmuf_(mmuf),
              nr_pages_(0),
              map_ctrl_(ctrl),
              total_mapped_(0)
        {}

        auto append(PhysAddr phys) -> Status;
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
    auto MappingCoalescer<MaxNumPages>::append(PhysAddr phys) -> Status {
        pa_[nr_pages_++] = phys;
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

    VmMapping::VmMapping(VmArea *parent, VirtAddr base, usize size, VmaFlags vmaf,
                         ustl::Rc<VmObject> vmo, usize vmo_off, const char *name)
        : Base(base, size, vmaf | VmaFlags::Mapping, parent, parent->aspace().as_ptr_mut(), name),
          vmo_off_(vmo_off), vmo_(ustl::move(vmo)), regions_()
    {}

    auto VmMapping::create(VmArea *parent, VirtAddr base, usize size, VmaFlags vmaf,
                           ustl::Rc<VmObject> vmo, usize vmo_off, MmuFlags mmuf, 
                           char const *name, ustl::Rc<Self> *out) -> Status {
        if (!vmo) {
            return Status::InvalidArguments;
        }

        auto mapping = new (*s_vm_mapping_cache, kGafKernel) VmMapping(
            parent, base, size, vmaf, ustl::move(vmo), vmo_off, name);

        if (!mapping) {
            return Status::OutOfMem; 
        }
        auto status = mapping->regions_.init(base + size, mmuf);
        if (Status::Ok != status) {
            s_vm_mapping_cache->deallocate(mapping);
            return status;
        }
        mapping->activate();

        *out = ustl::make_rc<Self>(mapping);
        return Status::Ok;
    }

    auto VmMapping::activate() -> Status {
        Base::activate();

        // Note when a mapping was added into a VMO's mapping list, the inside counter
        // has not been increased implicitly. It is required to pay attention to manually 
        // install and uninstall a mapping, avoiding lifecycle problem.
        vmo_->add_mapping(*this);

        return Status::Ok;
    }

    auto VmMapping::destroy() -> Status {
        canary_.verify();

        auto status = aspace_->arch_aspace().unmap(base_, size_ >> PAGE_SHIFT, {}, 0);
        if (Status::Ok != status) {
            log::trace("Failed to destroy Mapping with unmapping {} pages at {}", size_ >> PAGE_SHIFT, base_);
            return status;
        }

        if (vmo_) {
            vmo_->remove_mapping(*this);
        }
        Base::destroy();
        return Status::Ok;
    }

    FORCE_INLINE
    auto VmMapping::map_paged(VmObjectPaged *vmo, VirtAddr base, usize size, bool commit, MapControl control) -> Status {
        DEBUG_ASSERT(ustl::mem::is_aligned(base, PAGE_SIZE));
        DEBUG_ASSERT(ustl::mem::is_aligned(size, PAGE_SIZE));

        CXX11_CONSTEXPR
        static auto const kMaxBatchPages = 32;

        auto cursor = vmo->make_cursor(base, size);
        if (!cursor) {
            return cursor.unwrap_err();
        }

        auto enumerator = regions_.make_enumerator(base, size);
        while (auto region = enumerator.next()) {
            auto [base, size, mmuf] = *region;
            MappingCoalescer<kMaxBatchPages> coalescer(this, base, mmuf, control);

            PageRequest page_request;
            for (auto i = 0; i < size; i += PAGE_SIZE) {
                auto result = cursor->require_owned_page(1, &page_request);
                if (!result) {
                    return result.unwrap_err();
                }

                coalescer.append(frame_to_phys(*result));
            }
            // Commit those uncovered units in for loop above.
            coalescer.commit();
        }

        return Status::Ok;
    }

    FORCE_INLINE
    auto VmMapping::map_physical(VmObjectPhysical *vmo, VirtAddr base, usize size, MapControl control) -> Status {
        DEBUG_ASSERT(vmo);
        PhysAddr phys_base;
        auto status = vmo->lookup_range(base - base_, size, &phys_base);
        if (Status::Ok != status) {
            return status;
        }

        CXX11_CONSTEXPR
        static auto const kMaxBatchPages = 32;

        auto enumerator = regions_.make_enumerator(base, size);
        while (auto region = enumerator.next()) {
            auto [base, size, mmuf] = *region;
            MappingCoalescer<kMaxBatchPages> coalescer(this, base, mmuf, control);
            for (auto offset = 0; offset < size; offset += PAGE_SIZE) {
                coalescer.append(phys_base + offset);
            }

            coalescer.commit();
        }

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
        if (!check_range(base, size)) {
            return Status::InvalidArguments;
        }

        if (auto paged = downcast<VmObjectPaged>(vmo_.as_ptr_mut())) {
            return map_paged(paged, base, size, commit, control);
        } else if (auto physical = downcast<VmObjectPhysical>(vmo_.as_ptr_mut())) {
            return map_physical(physical, base, size, control);
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
        if (!check_range(base, size)) {
            return Status::InvalidArguments;
        }

        if (!validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        auto &arch_aspace = aspace_->arch_aspace();
        auto status = regions_.update(base, size, mmuf, 0, size);
        if (Status::Ok != status) {
            return status;
        }

        auto enumerator = regions_.make_enumerator(base, size);
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
        if (!check_range(base, size)) {
            return Status::InvalidArguments;
        }

        auto &arch_aspace = aspace_->arch_aspace();
        auto enumerator = regions_.make_enumerator(base, size);
        while (auto region = enumerator.next()) {
            auto [base, size, mmuf] = *region;
            auto status = arch_aspace.unmap(base, size >> PAGE_SHIFT, control, 0);
            if (Status::Ok != status) {
                log::error("Failed to unmap range[{:X}, {:X})", base, size);
                return status;
            }
        }

        return Status::Ok;
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

        s_vm_mapping_region_cache = ObjectCache::create<MappingRegionSet::Region>("vm-mapping-region-cache", OcFlags::Folio);
        if (!s_vm_mapping_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("MappingRegionCache has been created");
    }
    GKTL_INIT_HOOK(VmMappingCacheInit, init_vm_mapping_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::mem
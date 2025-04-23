#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/object-cache.hpp>

#include <logz4/log.hpp>

#include <ktl/new.hpp>
#include <gktl/init_hook.hpp>
#include <ustl/mem/align.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    static ObjectCache *s_vmo_paged_cache;

    VmObjectPaged::VmObjectPaged(VmoFLags vmof, ustl::Rc<VmCowPages> cowpages)
        : Base(Type::Paged, vmof),
          cow_pages_(ustl::move(cowpages))
    {}

    auto VmObjectPaged::create(Gaf gaf, usize nr_pages, VmoFLags vmof)
        -> ustl::Result<ustl::Rc<VmObjectPaged>, Status> {
        // Check vmof
        auto cow_pages = VmCowPages::create(gaf, nr_pages);
        if (!cow_pages) {
            return ustl::err(cow_pages.unwrap_err());
        }

        auto vmo = new (*s_vmo_paged_cache, kGafKernel) VmObjectPaged(vmof, ustl::move(cow_pages.unwrap()));
        if (!vmo) {
            return ustl::err(Status::OutOfMem);
        }

        /// A non-lazy VMO need we directly request pages it demands.
        if (!(vmof & VmoFLags::Lazy)) {
            CommitOptions options{};
            if (!!(vmof & VmoFLags::Pinned)) {
                options |= CommitOptions::Pin;
            }

            vmo->commit_range(0, nr_pages, options);
        }

        return ustl::ok(ustl::make_rc<Self>(vmo));
    }

    FORCE_INLINE
    auto VmObjectPaged::commit_range_internal(PgOff pgoff, usize n, CommitOptions option) -> Status {
        if (!!(option & CommitOptions::Pin)) {
            if (!n) {
                return Status::InvalidArguments;
            }
        }

        if (!n) {
            return Status::Ok;
        } 
        
        ustl::sync::LockGuard guard(mutex_);
        if (cow_pages_->num_pages_locked() < n) {
            return Status::OutOfRange;
        }

        while (n > 0) {
            usize nr_commited = 0;
            auto status = cow_pages_->commit_range_locked(pgoff, n, &nr_commited);

            if (status != Status::Ok && status != Status::ShouldWait) {
                return status;
            }
        }

        return Status::Ok;
    }

    auto VmObjectPaged::commit_range(PgOff pgoff, usize n, CommitOptions option) -> Status {
        canary_.verify();
        log::trace("VMO Commit Action: [pgoff: {:X}, n: {:X}]", pgoff, n);
        return commit_range_internal(pgoff, n, option);
    }

    auto VmObjectPaged::decommit(PgOff pgoff, usize n) -> Status {
        return Status::Unimplemented;
    }

    auto VmObjectPaged::take_pages(PgOff pgoff, usize n, VmPageList *pagelist) -> Status {
        return Status::Unimplemented;
    }

    auto VmObjectPaged::supply_pages(PgOff pgoff, usize n, VmPageList *pagelist) -> Status {
        return Status::Unimplemented;
    }

    auto VmObjectPaged::read(void *out, PgOff pgoff, usize size) -> Status { 
        return Status::Unimplemented;
    }

    auto VmObjectPaged::write(void *in, PgOff pgoff, usize size) -> Status { 
        return Status::Unimplemented;
    }

    INIT_CODE
    static auto init_vmo_paged_cache() -> void {
        s_vmo_paged_cache = ObjectCache::create<VmObjectPaged>("vmo-paged-cache", OcFlags::Folio);
        if (!s_vmo_paged_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("VmoPagedCache has been created");
    }
    GKTL_INIT_HOOK(VmoPagedCacheInit, init_vmo_paged_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::mem
#include <ours/mem/vm_cow_pages.hpp>
#include <ours/mem/object-cache.hpp>

#include <gktl/init_hook.hpp>

namespace ours::mem {
    static ustl::Rc<ObjectCache> s_vm_cow_pages_cache;

    VmCowPages::VmCowPages(Gaf gaf, usize nr_pages)
        : gaf_(gaf),
          num_pages_(nr_pages)
    {}

    auto VmCowPages::create(Gaf gaf, usize nr_pages) 
        -> ustl::Result<ustl::Rc<VmCowPages>, Status>
    {
        auto cow_pages = s_vm_cow_pages_cache->allocate<Self>(kGafKernel, gaf, nr_pages);
        if (!cow_pages) {
            return ustl::err(Status::OutOfMem);
        }

        return ustl::ok(ustl::make_rc<Self>(cow_pages));
    }

    auto VmCowPages::commit_range_locked(PgOff pgoff, usize n, ai_out usize *nr_commited) -> Status {
        if (!n) {
            return Status::InvalidArguments;
        }

        if (pgoff + n > num_pages_) {
            return Status::InvalidArguments;
        }

        return Status::Ok;
    }

    auto VmCowPages::Cursor::require_owned_page(usize nr_pages, PageRequest *page_request)
        -> ustl::Result<VmPage *, Status> {
        return ustl::ok(nullptr);
    }

    INIT_CODE
    static auto init_vm_cow_pages_cache() -> void {
        s_vm_cow_pages_cache = ObjectCache::create<VmCowPages>("vmo-paged-cache", OcFlags::Folio);
        if (!s_vm_cow_pages_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("VmCowPagesCache has been created");
    }
    GKTL_INIT_HOOK(VmCowPagesCacheInit, init_vm_cow_pages_cache, gktl::InitLevel::PlatformEarly);

} // namespace ours::mem
#include <ours/mem/vm_cow_pages.hpp>
#include <ours/mem/object-cache.hpp>
#include <ours/mem/pmm.hpp>

#include <gktl/init_hook.hpp>
#include <ktl/new.hpp>

namespace ours::mem {
    static ObjectCache *s_vm_cow_pages_cache;

    VmCowPages::VmCowPages(Gaf gaf, usize nr_pages)
        : gaf_(gaf),
          num_pages_(nr_pages)
    {}

    auto VmCowPages::create(Gaf gaf, usize nr_pages, ustl::Rc<VmCowPages> *out) -> Status {
        auto cow_pages = new (*s_vm_cow_pages_cache, kGafKernel) Self(gaf, nr_pages);
        if (!cow_pages) {
            return Status::OutOfMem;
        }
        *out = cow_pages;

        return Status::Ok;
    }

    auto VmCowPages::make_cursor(PgOff pgoff, usize nr_pages) -> ustl::Result<Cursor, Status> {
        return ustl::ok(Cursor(this, pgoff, nr_pages));
    }

    auto VmCowPages::alloc_pages(usize order, VmPage **page, PageRequest *page_request) -> Status {
        auto frame = alloc_frame(gaf_, order);
        if (!frame) {
            return Status::OutOfMem;
        }
        *page = role_cast<PfRole::Vmm>(frame);
        return Status::Ok;
    }

    auto VmCowPages::commit_range_locked(PgOff pgoff, usize n, ai_out usize *nr_commited) -> Status {
        if (!n) {
            return Status::InvalidArguments;
        }

        if (pgoff + n > num_pages_) {
            return Status::InvalidArguments;
        }

        auto cursor = make_cursor(pgoff, n);
        if (!cursor) {
            return Status::InternalError;
        }

        auto status = Status::Ok;
        auto commited = 0;
        PageRequest page_request;
        for (auto commited = 0; commited < n; ++commited) {
            auto result = cursor->require_owned_page(1, &page_request);
            if (!result) {
                status = result.unwrap_err();
                DEBUG_ASSERT(status != Status::ShouldWait);
                break;
            }
        }

        if (nr_commited) {
            *nr_commited = commited;
        }

        return status;
    }

    /// The followings are in class VmCowPages::Cursor.

    VmCowPages::Cursor::Cursor(VmCowPages *cow_pages, PgOff pgoff, usize nr_pages)
        : owner_(cow_pages) 
    {}

    auto VmCowPages::Cursor::require_owned_page(usize nr_pages, PageRequest *page_request)
        -> ustl::Result<VmPage *, Status> {
        auto status = owner_->alloc_pages(0, 0, page_request);
        if (Status::Ok == status) {
            return ustl::ok(nullptr);
        }

        // No any available pages for current request, try to create asynchronous page request.
        return ustl::err(create_read_request(nr_pages, page_request));
    }

    auto VmCowPages::Cursor::create_read_request(usize nr_pages, PageRequest *page_request) -> Status {
        return Status::Unimplemented;
    }

    auto VmCowPages::Cursor::create_dirty_request(usize nr_pages, PageRequest *page_request) -> Status {
        return Status::Unimplemented;
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
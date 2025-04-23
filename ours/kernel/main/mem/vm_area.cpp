#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object.hpp>
#include <ours/mem/object-cache.hpp>

#include <ktl/new.hpp>
#include <gktl/init_hook.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    VmArea::VmArea(ustl::Rc<VmAspace> aspace, 
                   VirtAddr base, 
                   usize size, 
                   VmaFlags vmaf, 
                   char const *name)
        : canary_(), base_(base), size_(size), vmaf_(vmaf), 
          name_(name), subvmas_(), mapping_(), aspace_(aspace),
          subvma_hook_()
    {}

    static ObjectCache *s_vma_cache;

    auto VmArea::create(ustl::Rc<VmAspace> aspace, VirtAddr base, usize size, 
                        VmaFlags vmaf, char const *name, ustl::Rc<Self> *out) -> Status {
        DEBUG_ASSERT(aspace, "Given a invalid aspace");
        auto self = new (*s_vma_cache, kGafKernel) Self(aspace, base, size, vmaf, name);
        if (!self) {
            return Status::OutOfMem;
        }
        *out = ustl::make_rc<VmArea>(self);

        return Status::Ok;
    }

    auto VmArea::activate() -> void {
        vmaf_ |= VmaFlags::Active;
    }

    FORCE_INLINE
    auto VmArea::has_range(VirtAddr base, usize size) const -> bool {
        auto prev = subvmas_.upper_bound(base, [] (VirtAddr base, Self const &x) {
            return base < x.base_;
        });

        if (prev == subvmas_.end()) {
            return false;
        }
        auto next = prev--;
        if (next->base_ < base + size && prev->base_ + prev->size_ < base) {
            return false;
        }

        return true;
    }

    FORCE_INLINE
    auto VmArea::prepare_subrange(PgOff vma_off, usize nr_pages, VirtAddr ai_out &base, VirtAddr ai_out &size) 
        const -> bool {
        if (!nr_pages) {
            return false;
        }

        // Now vbase is offset but real base address.
        VirtAddr vbase = (vma_off << PAGE_SHIFT);
        VirtAddr vsize = (nr_pages << PAGE_SHIFT);
        if (vbase + vsize > size_) {
            return false;
        }
        vbase += base_;

        if (has_range(vbase, vsize)) {
            return false;
        }

        base = vbase;
        size = vsize;

        return true;
    }

    FORCE_INLINE
    auto VmArea::validate_mmuflags(MmuFlags mmuf) const -> bool {
        VmaFlags vmaf{};
        if (!!(mmuf & MmuFlags::Readable)) {
            vmaf |= VmaFlags::Read;
        }
        if (!!(mmuf & MmuFlags::Writable)) {
            vmaf |= VmaFlags::Write;
        }
        if (!!(mmuf & MmuFlags::Executable)) {
            vmaf |= VmaFlags::Exec;
        }

        auto rights_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | rights_allowed) != rights_allowed) {
            return false;
        }

        return true;
    }

    auto VmArea::create_subvma(usize nr_pages, VmaFlags vmaf, char const *name, ustl::Rc<VmArea> *out) -> Status {
        PgOff pgoff = 0;
        // Find a fixed area
        return create_subvma(pgoff, nr_pages, vmaf, name, out);
    }

    auto VmArea::create_subvma(PgOff vma_off, usize nr_pages, VmaFlags vmaf, char const *name, ustl::Rc<VmArea> *out)
        -> Status {
        DEBUG_ASSERT(!mapping_, "Attempt to create a sub-area for a terminal VMA");

        VirtAddr base, size;
        if (!prepare_subrange(vma_off, nr_pages, base, size)) {
            return Status::InvalidArguments;
        }

        auto rights_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | rights_allowed) != rights_allowed) {
            return Status::InvalidArguments;
        }

        // The other flags of sub-VMA also be restricted under parent VMA.
        vmaf &= vmaf_;
        auto status = VmArea::create(aspace_, base, size, vmaf, name, out);
        if (Status::Ok != status) {
            return status;
        }

        return Status::Ok;
    }

    auto VmArea::create_mapping(PgOff vma_off, usize nr_pages, 
                                PgOff vmo_off, MmuFlags mmuf, ustl::Rc<VmObject> vmo,
                                char const *name, ustl::Rc<VmMapping> *out) -> Status {
        DEBUG_ASSERT(subvmas_.empty(), "Attempt to create mapping for a interminal VMA");

        // First, we check if the given MMU flags have the same or lower permissions than 
        // those allowed by this VMA.
        if (validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        VirtAddr base, size;
        if (!prepare_subrange(vma_off, nr_pages, base, size)) {
            return Status::InvalidArguments;
        }

        ustl::Rc<VmMapping> mapping;
        auto status = VmMapping::create(base, size, this, vma_off, vmo, mmuf, name, &mapping);
        if (Status::Ok != status) {
            return status;
        }
        mapping->activate();
        *out = ustl::move(mapping);

        return Status::Ok;
    } 

    auto VmArea::unmap(PgOff vma_off, usize nr_pages) -> void {
    }


    auto VmArea::protect(PgOff vma_off, usize nr_pages, MmuFlags flags) -> void {
    }

    INIT_CODE
    static auto init_vma_cache() -> void {
        s_vma_cache = ObjectCache::create<VmArea>("vma-cache", OcFlags::Folio);
        if (!s_vma_cache) {
            panic("Failed to create object cache for VmArea");
        }
        log::trace("VmaCache has been created");
    }
    GKTL_INIT_HOOK(VmCacheInit, init_vma_cache, gktl::InitLevel::PlatformEarly);
}
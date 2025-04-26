#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object.hpp>
#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/object-cache.hpp>

#include <ktl/new.hpp>
#include <gktl/init_hook.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    VmArea::VmArea(VirtAddr base, usize size, VmaFlags vmaf, Self *parent, VmAspace *aspace, char const *name)
        : Base(base, size, vmaf, parent, aspace, name)
    {}

    static ObjectCache *s_vma_cache;

    auto VmArea::create(VirtAddr base, usize size, VmaFlags vmaf, Self *parent, VmAspace *aspace,
                        char const *name, ustl::Rc<Self> *out) -> Status {
        DEBUG_ASSERT(aspace, "Given a invalid aspace");
        auto self = new (*s_vma_cache, kGafKernel) Self(base, size, vmaf, parent, aspace, name);
        if (!self) {
            return Status::OutOfMem;
        }
        *out = ustl::move(ustl::make_rc<VmArea>(self));

        return Status::Ok;
    }

    auto VmArea::activate() -> void {
        vmaf_ |= VmaFlags::Active;
    }

    auto VmArea::create_subvma(usize nr_pages, VmaFlags vmaf, char const *name, ustl::Rc<VmArea> *out) -> Status {
        canary_.verify();
        auto result = subvmas_.find_spot(nr_pages, PAGE_SIZE, base_, size_);
        if (!result) {
            return result.unwrap_err();
        }
        // Find a fixed area
        return create_subvma(*result - base_, nr_pages, vmaf, name, out);
    }

    auto VmArea::create_subvma(usize vma_ofs, usize size_bytes, VmaFlags vmaf, char const *name, ustl::Rc<VmArea> *out)
        -> Status {
        canary_.verify();

        auto [base, size] = resolve_page_range(vma_ofs, size_bytes);
        if (subvmas_.has_range(base, size)) {
            return Status::InvalidArguments;
        }

        auto perm_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | perm_allowed) != perm_allowed) {
            return Status::InvalidArguments;
        }

        // The other flags of sub-VMA also be restricted under parent VMA.
        vmaf &= vmaf_;
        auto status = VmArea::create(base, size, vmaf, this, aspace_.as_ptr_mut(), name, out);
        if (Status::Ok != status) {
            return status;
        }

        return Status::Ok;
    }

    auto VmArea::create_mapping(usize vma_ofs, usize size_bytes, 
                                usize vmo_ofs, MmuFlags mmuf, ustl::Rc<VmObject> vmo,
                                char const *name, ustl::Rc<VmMapping> *out) -> Status {
        DEBUG_ASSERT(subvmas_.empty(), "Attempt to create mapping for a interminal VMA");
        canary_.verify();

        auto [base, size] = resolve_page_range(vma_ofs, size_bytes);

        // First, we check if the given MMU flags have the same or lower permissions than 
        // those allowed by this VMA.
        if (validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        if (subvmas_.has_range(base, size)) {
            return Status::InvalidArguments;
        }

        ustl::Rc<VmMapping> mapping;
        auto status = VmMapping::create(this, base, size, vmaf_, vmo, vmo_ofs, mmuf, name, &mapping);
        if (Status::Ok != status) {
            return status;
        }
        mapping->activate();
        *out = ustl::move(mapping);

        return Status::Ok;
    } 

    auto VmArea::unmap(usize offset, usize size) -> Status {
        return Status::Unimplemented; 
    }


    auto VmArea::protect(usize offset, usize size, MmuFlags flags) -> Status {
        return Status::Unimplemented;
    }

    auto VmArea::reserve(usize vma_off, usize size_bytes, MmuFlags mmuf, char const *name) -> Status {
        canary_.verify();
        auto [base, size] = resolve_page_range(base_ + vma_off, size_bytes);
        if (!size) {
            return Status::InvalidArguments;
        }

        if (!validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        // TODO(SmallHuaZi) Maybe we should provide a specific way to make these kind of zero length VMO.
        ustl::Rc<VmObjectPaged> vmo;
        auto status = VmObjectPaged::create(kGafKernel, 0, VmoFLags::Pinned, &vmo);
        if (Status::Ok != status) {
            return status;
        }

        ustl::Rc<VmMapping> mapping;
        status = VmMapping::create(this, base, size, vmaf_,vmo,  0, mmuf, name, &mapping);
        if (Status::Ok != status) {
            return status;
        }

        // The mappings are existing, just update the permission of them.
        status = aspace_->arch_aspace().protect(base, size >> PAGE_SHIFT, mmuf);
        return status;
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
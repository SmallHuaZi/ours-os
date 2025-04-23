#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object.hpp>
#include <ours/mem/object-cache.hpp>

#include <gktl/init_hook.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    CXX11_CONSTEXPR
    static VmaFlags const kVmaFlagsInitAllowed = VmaFlags::Mergeable | 
                                                 VmaFlags::Anonymous |
                                                 VmaFlags::MayExec |
                                                 VmaFlags::MayRead |
                                                 VmaFlags::MayWrite |
                                                 VmaFlags::MayShare;

    VmArea::VmArea(ustl::Rc<VmAspace> aspace, 
                   VirtAddr base, 
                   usize size, 
                   VmaFlags flags, 
                   char const *name)
        : Base(base, size, flags, name),
          subvmas_()
    {}

    static ustl::Rc<ObjectCache> s_vma_cache;

    auto VmArea::create(ustl::Rc<VmAspace> aspace, VirtAddr base, usize size, 
                        VmaFlags vmaf, char const *name) 
        -> ustl::Result<ustl::Rc<Self>, Status> {
        DEBUG_ASSERT(aspace, "Given a invalid aspace");
        auto self = s_vma_cache->allocate<Self>(aspace, base, size, vmaf, name);
        if (!self) {
            return ustl::err(Status::OutOfMem);
        }

        return ustl::ok(ustl::make_rc<VmArea>(self));
    }

    auto VmArea::activate() -> void {
        vmaf_ |= VmaFlags::Active;
    }

    FORCE_INLINE
    auto VmArea::prepare_create_subvma(PgOff vma_off, usize nr_pages, VmaFlags &vmaf, 
                                       VirtAddr ai_out &base, VirtAddr ai_out &size)  const -> bool {
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

        if (subvmas_.has_range(vbase, vsize)) {
            return false;
        }

        base = vbase;
        size = vsize;

        auto rights_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | rights_allowed) != rights_allowed) {
            return false;
        }

        // The other flags of sub-VMA also be restricted under parent VMA.
        vmaf &= vmaf_;
        return true;
    }

    FORCE_INLINE
    auto VmArea::validate_mmuflags(MmuFlags mmuf) const -> VmaFlags {
        VmaFlags vmaf{};
        if (!!(mmuf & MmuFlags::Readable)) {
            vmaf |= VmaFlags::MayRead | VmaFlags::Read;
        }
        if (!!(mmuf & MmuFlags::Writable)) {
            vmaf |= VmaFlags::MayWrite | VmaFlags::Write;
        }
        if (!!(mmuf & MmuFlags::Executable)) {
            vmaf |= VmaFlags::MayExec | VmaFlags::Exec;
        }

        auto rights_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | rights_allowed) != rights_allowed) {
            return VmaFlags::None;
        }

        return vmaf;
    }

    auto VmArea::create_subvma(PgOff vma_off, usize nr_pages, VmaFlags vmaf, char const *name)
        -> ustl::Result<ustl::Rc<VmArea>, Status> {
        VirtAddr base, size;
        if (!prepare_create_subvma(vma_off, nr_pages, vmaf, base, size)) {
            return ustl::err(Status::InvalidArguments);
        }

        auto result = VmArea::create(aspace_, base, size, vmaf, name);
        if (!result) {
            return ustl::err(result.unwrap_err());
        }

        return ustl::ok(ustl::move(result.unwrap()));
    }

    auto VmArea::create_mapping(PgOff vma_off, usize nr_pages, VmaFlags vmaf, 
                                PgOff vmo_off, MmuFlags mmuf, ustl::Rc<VmObject> vmo, 
                                char const *name) 
        -> ustl::Result<ustl::Rc<VmMapping>, Status> {
        auto const perms = validate_mmuflags(mmuf);
        if (perms == VmaFlags::None) {
            return ustl::err(Status::InvalidArguments);
        }
        vmaf |= perms;

        VirtAddr base, size;
        if (!prepare_create_subvma(vma_off, nr_pages, vmaf, base, size)) {
            return ustl::err(Status::InvalidArguments);
        }

        auto result = VmMapping::create(base, size, this, vmaf, vma_off, vmo, mmuf, name);
        if (!result) {
            return ustl::err(result.unwrap_err());
        }
        ustl::Rc<VmMapping> mapping(result.unwrap());

        return ustl::ok(mapping);
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
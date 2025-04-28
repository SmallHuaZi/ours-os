#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object.hpp>
#include <ours/mem/vm_object_paged.hpp>
#include <ours/mem/vm_object_physical.hpp>
#include <ours/mem/object-cache.hpp>

#include <logz4/log.hpp>
#include <ktl/new.hpp>
#include <gktl/init_hook.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace ours::mem {
    VmArea::VmArea(VirtAddr base, usize size, VmaFlags vmaf, Self *parent, VmAspace *aspace, char const *name)
        : Base(base, size, vmaf, parent, aspace, name), 
          subvmas_()
    {}

    static ObjectCache *s_vma_cache;

    auto VmArea::create_common(VirtAddr base, usize size, VmaFlags vmaf, VmArea *parent, 
                               VmAspace *aspace, char const *name, ustl::Rc<Self> *out) -> Status {
        DEBUG_ASSERT(aspace, "Given a invalid aspace");
        DEBUG_ASSERT(out, "`out` can not be null");

        auto self = new (*s_vma_cache, kGafKernel) Self(base, size, vmaf, parent, aspace, name);
        if (!self) {
            return Status::OutOfMem;
        }
        if (!parent) {
            // A root VMA has no parent so just mark it active.
            self->mark_active();
        } else {
            self->activate();
        }

        *out = ustl::make_rc<VmArea>(self);
        return Status::Ok;
    }

    struct VmArea::CreateVmAomArgs {
        VirtAddr base;
        usize size;
        VmaFlags vmaf;
        VmMapOption option;
        ustl::Rc<VmObject> vmo;
        usize vmo_ofs;
        MmuFlags mmuf;
        char const *name;
    };

    auto VmArea::create_subaom_internal(CreateVmAomArgs &packet, ustl::Rc<VmAreaOrMapping> *out) -> Status {
        DEBUG_ASSERT(out, "`out` must not be null");

        if (!is_active()) {
            // Operate a illegal VMA.
            return Status::BadState;
        }
        if (!validate_vmaflags(packet.vmaf)) {
            // Unallowed permissions.
            return Status::InvalidArguments;
        }
        if (!check_range(packet.base, packet.size)) {
            // Not in this VMA 
            return Status::InvalidArguments;
        }

        auto const is_fixed_noreplace = (!!(packet.option & VmMapOption::FixedNoReplace));
        auto const is_fixed = (!!(packet.option & VmMapOption::Fixed)) || is_fixed_noreplace;

        if (is_fixed) {
            if (subvmas_.has_range(packet.base, packet.size)) {
                if (is_fixed_noreplace) {
                    return Status::AlreadyExists;
                }

                // Overwrite the primitive regions.
                DEBUG_ASSERT(false, "Now we don't support overwrite");
            }
        } else {
            auto result = subvmas_.find_spot(packet.base, PAGE_SIZE, base_, base_ + size_);
            if (!result) {
                return Status::InvalidArguments;
            }

            packet.base = result.unwrap();
        }

        if (packet.vmo) {
            ustl::Rc<VmMapping> mapping;
            auto status = VmMapping::create(this, packet.base, packet.size, packet.vmaf, packet.vmo, 
                                           packet.vmo_ofs, packet.mmuf, packet.name, &mapping);
            if (Status::Ok != status) {
                return status;
            }
            *out = ustl::make_rc<VmAreaOrMapping>(ustl::move(mapping));
        } else {
            ustl::Rc<VmArea> vma;
            auto status = VmArea::create(packet.base, packet.size, packet.vmaf, this, aspace_.as_ptr_mut(), packet.name, &vma);
            if (Status::Ok != status) {
                return status;
            }
            *out = ustl::make_rc<VmAreaOrMapping>(ustl::move(vma));
        }

        return Status::Ok;
    }

    auto VmArea::create_subvma(usize vma_ofs, usize size, VmaFlags vmaf, char const *name, 
                               VmMapOption option, ustl::Rc<VmArea> *out) -> Status {
        canary_.verify();

        auto [base_aligned, size_aligned] = resolve_page_range(base_ + vma_ofs, size);
        CreateVmAomArgs packet{
            .base = base_aligned,
            .size = size_aligned,
            .vmaf = vmaf,
            .option = option,
            .name = name
        };
        ustl::Rc<VmAreaOrMapping> aom;
        auto status = create_subaom_internal(packet, &aom);
        if (Status::Ok != status) {
            return status;
        }
        *out = ustl::downcast<VmArea>(&aom);

        return Status::Ok;
    }

    auto VmArea::create_mapping(usize vma_ofs, usize size, usize vmo_ofs, MmuFlags mmuf, 
                                ustl::Rc<VmObject> vmo, char const *name, VmMapOption option,
                                ustl::Rc<VmMapping> *out) -> Status {
        canary_.verify();

        if (!size) {
            return Status::InvalidArguments;
        }

        // Check if the given MMU flags have the same or lower permissions than 
        // those allowed by this VMA.
        if (!validate_mmuflags(mmuf)) {
            return Status::InvalidArguments;
        }

        auto const [base_aligned, size_aligned] = resolve_page_range(base_ + vma_ofs, size);
        CreateVmAomArgs packet{
            .base = base_aligned,
            .size = size_aligned,
            .vmaf = vmaf_,
            .option = option,
            .vmo = ustl::move(vmo),
            .vmo_ofs = vmo_ofs,
            .mmuf = mmuf,
            .name = name
        };
        ustl::Rc<VmAreaOrMapping> aom;
        auto status = create_subaom_internal(packet, &aom);
        if (Status::Ok != status) {
            return status;
        }
        *out = ustl::downcast<VmMapping>(&aom);
        num_mappings_ += 1;

        return Status::Ok;
    }

    auto VmArea::map_at(PhysAddr phys_base, VirtAddr virt_base, usize size, MmuFlags mmuf, 
                        VmMapOption options, char const *name) -> ktl::Result<ustl::Rc<VmMapping>> {
        DEBUG_ASSERT(ustl::mem::is_aligned(phys_base, PAGE_SIZE));
        auto [base_aligned, size_aligned] = resolve_page_range(virt_base, size);
        if (!size_aligned) {
            return ustl::err(Status::InvalidArguments);
        }

        ustl::Rc<VmObjectPhysical> vmo;
        auto status = VmObjectPhysical::create(phys_base, size_aligned, VmoFLags::Pinned, &vmo);
        if (Status::Ok != status) {
            return ustl::err(status);
        }

        return map_with_vmo(base_aligned, size_aligned, mmuf, ustl::move(vmo), options, name);
    }

    auto VmArea::map(VirtAddr base, usize size, MmuFlags mmuf, VmMapOption options, 
                     char const *name) -> ktl::Result<ustl::Rc<VmMapping>> {
        DEBUG_ASSERT(!(options & VmMapOption::Fixed), "Now we don't support the overwritting option");

        auto [base_aligned, size_aligned] = resolve_page_range(base, size);
        if (!size_aligned) {
            return ustl::err(Status::InvalidArguments);
        }

        ustl::Rc<VmObjectPaged> vmo;
        auto status = VmObjectPaged::create(kGafKernel, size_aligned, VmoFLags::Pinned, &vmo);
        if (Status::Ok != status) {
            return ustl::err(status);
        }

        return map_with_vmo(base_aligned, size_aligned, mmuf, ustl::move(vmo), options, name);

    }

    auto VmArea::map_with_vmo(VirtAddr base, usize size, MmuFlags mmuf, ustl::Rc<VmObject> vmo, 
                              VmMapOption option, char const *name) -> ktl::Result<ustl::Rc<VmMapping>> {
        ustl::Rc<VmMapping> mapping;
        auto status = create_mapping(base - base_, size, 0, mmuf, ustl::move(vmo), name, option, &mapping);
        if (Status::Ok != status) {
            return ustl::err(status);
        }

        if (!!(VmMapOption::Commit & option)) {
            vmo->commit_range(0, size);

            status = mapping->map(0, size, true, MapControl::ErrorIfExisting);
            if (Status::Ok != status) {
                return ustl::err(status);
            }
        }

        return ustl::ok(ustl::move(mapping)); 
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

        // TODO(SmallHuaZi) Should we provide a specific way to make these kind of zero length VMO.
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

    auto VmArea::dump() const -> void {
        log::trace("VMA {}: [0x{:X}, 0x{:X}]", name_, base_, size_);
        for (auto &child : subvmas_) {
            if (child.is_mapping()) {
                log::trace("  M {:<10}: [0x{:X}, 0x{:X}]", child.name_, child.base(), child.size());
            } else {
                log::trace("  A {:<10}: [0x{:X}, 0x{:X}]", child.name_, child.base(), child.size());
            }
        }
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
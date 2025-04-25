#include <ours/mem/vmm.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_object_paged.hpp>

namespace ours::mem {
    NO_MANGLE {
        VmPage *g_zero_page;
        PhysAddr g_kernel_phys_base;
        VirtAddr g_kernel_virt_base;
    }

    auto switch_context(VmAspace *from, VmAspace *to) -> void {
        DEBUG_ASSERT(to != nullptr);
        VmAspace::switch_aspace(from, to);
    }

    auto set_active_aspace(VmAspace *aspace) -> void {
        DEBUG_ASSERT(aspace != nullptr);
        VmAspace::switch_aspace(nullptr, aspace);
    }

    auto virt_to_phys(VirtAddr virt_addr) -> PhysAddr {
        auto phys_addr = PhysMap::virt_to_phys(virt_addr);
        if (!phys_addr) {
            VmAspace::kernel_aspace()->arch_aspace().query(virt_addr, &phys_addr, 0);
        }

        return phys_addr;
    }

    auto phys_to_virt(PhysAddr phys_addr) -> VirtAddr {
        return PhysMap::phys_to_virt(phys_addr);
    }

    /// This should be a common routine to extract MMU permissions from
    /// a VmaFlags.
    FORCE_INLINE
    static auto extract_permissions(VmaFlags vmaflags) -> MmuFlags {
        MmuFlags mmuf{};
        if (!!(vmaflags & VmaFlags::Write)) {
            mmuf |= MmuFlags::Writable;
        }
        if (!!(vmaflags & VmaFlags::Read)) {
            mmuf |= MmuFlags::Readable;
        }
        if (!!(vmaflags & VmaFlags::Exec)) {
            mmuf |= MmuFlags::Executable;
        }

        return mmuf;
    };

    auto vmmap(usize nr_pages, VmaFlags vmaf, const char *name, VmMapOption options) -> ustl::Result<VirtAddr, Status> {
        auto root_vma = VmAspace::kernel_aspace()->root_vma();

        ustl::Rc<VmArea> vma;
        auto status = root_vma->create_subvma(nr_pages, vmaf, name, &vma);
        if (Status::Ok != status) {
            return ustl::err(status);
        }

        VmoFLags vmof{};
        if (!(options & VmMapOption::Commit)) {
            vmof |= VmoFLags::Lazy;
        }
        if (!!(options & VmMapOption::Pinned)) {
            vmof |= VmoFLags::Pinned;
        }

        ustl::Rc<VmObjectPaged> vmo;
        status = VmObjectPaged::create(kGafKernel, nr_pages, vmof, &vmo);
        if (Status::Ok != status) {
            vma.destory();
            return ustl::err(status);
        }

        auto const mmuf = extract_permissions(vmaf);

        ustl::Rc<VmMapping> mapping;
        status = vma->create_mapping(0, nr_pages << PAGE_SHIFT, 0, mmuf, vmo, name, &mapping);

        return ustl::ok(vma->base());
    }

} // namespace ours::mem
#include <ours/arch/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <ours/init.hpp>

#include <logz4/log.hpp>

#include <arch/system.hpp>
#include <arch/tlb.hpp>

namespace ours::mem {
    /// Take out 2MB-size large page mapping as the following showed:
    /// +-------+---------+----------------+-------------------------+----------+-------+
    /// | Level | Table  | Size  | Range   | Bits     | Entries | Pages                 |
    /// +-------+--------+-----------------+-------------------------+----------+-------+
    /// | 0     | (page) | -     | 2 MiB   | 21 bits  | -       | 0x1 (1)               |
    /// +-------+--------+-----------------+-------------------------+----------+-------+
    /// | 2     | PD     | 4 KiB | 1 GiB   | 9 bits   |   512   | 0x200 (512)           |
    /// +-------+--------+-----------------+-------------------------+----------+-------+
    /// | 3     | PDP    | 4 KiB | 512 GiB-| 9 bits   |   512   | 0x40000 (262144)      |
    /// +-------+--------+-----------------+-------------------------+----------+-------+
    /// | 4     | PML4   | 4 KiB | 256 TiB-| 9 bits   |   512   | 0x8000000 (134217728) |
    /// +-------+--------+-----------------+-------------------------+----------+-------+
    ///
    /// The layout above is kernel address space by default.
    ///
    ///      (32GB) [0]--g_pdp[0]--g_physmap_pd[0..511]
    ///              |
    ///              |
    /// g_kernel_pgd--[..]-(NULL)
    ///              |                [0]---g_physmap_pd[0][0..511]
    ///              |                 |
    ///    (512GB) [511]-g_pdp---g_physmap_pd[..][0..511]
    ///                                |
    ///                               [31]---g_physmap_pd[31][0..511]
    ///

    /// In this file scope, it should be readonly.
    NO_MANGLE PhysAddr const g_kernel_pgd;

    ArchVmAspace::ArchVmAspace(VirtAddr base, usize size, VmasFlags flags)
        : range_(base, size),
          flags_(flags)
    {}

    auto ArchVmAspace::init() -> Status {
        canary_.verify();
        if (bool(VmasFlags::Kernel & flags_)) [[unlikely]] {
            // Kernel page table was provided by kernel.phys, so do not need to allocate memory.
            auto const phys_pgd = g_kernel_pgd;
            auto const virt_pgd = PhysMap::phys_to_virt(phys_pgd);
            auto status = page_table_.init_mmu(phys_pgd, virt_pgd);
            if (status != Status::Ok) {
                return status;
            }
        } else if (bool(VmasFlags::Guest & flags_)) {
            auto status = page_table_.init_ept();
            if (status != Status::Ok) {
                return status;
            }
        } else  [[likely]] {
            auto status = page_table_.init_mmu();
            if (status != Status::Ok) {
                return status;
            }
            // We should create an alias of kernel address space to avoid unnecessary switches.
            // And it could help us to call the routine passed by the driver of the user space correctly.
            auto &kpt = VmAspace::kernel_aspace()->arch_aspace().page_table_;
            page_table_.alias_to(kpt, PhysMap::kVirtBase, PhysMap::kSize / PAGE_SIZE);
        }

        return Status::Ok;
    }

    auto ArchVmAspace::switch_context(Self *from, Self *to) -> void {
        using arch::Cr3;
        if (to) {
            to->canary_.verify();
            VirtAddr const to_pgd = to->page_table_.pgd_phys();

            if (to->pcid_ != arch::kInvalidApicId) {
                Cr3::read().set<Cr3::Pcid>(to->pcid_)
                           .set<Cr3::PageTableAddress>(to_pgd)
                           .write();
            } else {
                Cr3::write(to_pgd);
            }
        } else {
            // Default to kernel aspace
            log::trace("From uaspace 0x{:X} switch to kaspace", from->page_table_.pgd_phys());
            Cr3::write(g_kernel_pgd);
        }

        // Finally, exchange the IO-Bitmap in TSS
    }

    auto ArchVmAspace::map(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control, usize *mapped) 
        -> Status {
        canary_.verify();
        return page_table_.map_pages(va, pa, n, flags, control, mapped);
    }

    auto ArchVmAspace::map_bulk(VirtAddr va, PhysAddr *pa, usize n, MmuFlags flags, MapControl control, usize *mapped) 
        -> Status {
        canary_.verify();
        return page_table_.map_pages_bulk(va, pa, n, flags, control, mapped);
    }

    auto ArchVmAspace::unmap(VirtAddr va, usize n, UnmapControl control, usize *unmapped) -> Status {
        canary_.verify();
        return page_table_.unmap_pages(va, n, control, unmapped);
    }

    auto ArchVmAspace::query(VirtAddr va, ai_out PhysAddr *pa, MmuFlags *flags) -> Status {
        canary_.verify();
        return page_table_.query_mapping(va, pa, flags);
    }

    auto ArchVmAspace::protect(VirtAddr va, usize n, MmuFlags mmuf) -> Status {
        canary_.verify();
        return page_table_.protect_pages(va, n, mmuf);
    }
}
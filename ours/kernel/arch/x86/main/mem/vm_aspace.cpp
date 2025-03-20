#include <ours/arch/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <ours/init.hpp>

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
    ///      (32GB) [0]--KERNEL_PDP[0]--KERNEL_PHYSMAP_PD[0..511]
    ///              |
    ///              | 
    /// KERNEL_PGD--[..]-(NULL)
    ///              |                [0]---KERNEL_PHYSMAP_PD[0][0..511]
    ///              |                 |
    ///    (512GB) [511]-KERNEL_PDP---ERNEL_PHYSMAP_PD[..][0..511]
    ///                                |
    ///                               [31]---KERNEL_PHYSMAP_PD[31][0..511]
    ///

    /// In this file scope, it should be readonly.
    NO_MANGLE PhysAddr const g_pgd;

    ArchVmAspace::ArchVmAspace(VirtAddr base, usize size, VmasFlags flags)
        : range_(base, size),
          flags_(flags)
    {}

    auto ArchVmAspace::init() -> Status
    {
        if (bool(VmasFlags::Kernel & flags_)) [[unlikely]] {
            // Kernel page table was provided by kernel.phys, so do not need to allocate memory. 
            auto const phys_pgd = g_pgd;
            auto const virt_pgd = PhysMap::phys_to_virt(phys_pgd );
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
            page_table_.alias_to(kpt, PhysMap::VIRT_BASE, PhysMap::SIZE / PAGE_SIZE);
        }

        return Status::Ok;
    }

    auto ArchVmAspace::map(VirtAddr va, PhysAddr pa, usize n, MmuFlags flags, MapControl control) -> ustl::Result<usize, Status>
    {  
        return page_table_.map_pages(va, pa, n, flags, control);
    }

    auto ArchVmAspace::unmap(VirtAddr va, usize n, UnMapControl control) -> Status
    {  
        return page_table_.unmap_pages(va, n, control);
    }

    auto ArchVmAspace::query(VirtAddr va, ai_out PhysAddr *pa, MmuFlags *flags) -> Status
    {
        return page_table_.query_mapping(va, pa, flags);
    }
}
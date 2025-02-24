#include <ours/arch/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/init.hpp>

#include <arch/x86/paging_traits.hpp>

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

    NO_MANGLE
    alignas(PAGE_SIZE) arch::Pte KERNEL_PGD[512]; // PML4

    NO_MANGLE
    alignas(PAGE_SIZE) arch::Pte KERNEL_PDP[512]; // PDP for the first 1GB

    NO_MANGLE
    alignas(PAGE_SIZE) arch::Pte KERNEL_PD[512]; // PD for the first 1GB

    NO_MANGLE
    alignas(PAGE_SIZE) arch::Pte KERNEL_PDP_HIGH[512]; // High PDP for the kernel address space 

    NO_MANGLE
    alignas(PAGE_SIZE) arch::Pte KERNEL_PHYSMAP_PD[PhysMap::SIZE / MAX_PAGE_SIZE]; //

    ///           KERNEL_PGD 
    ///       |0| ... || ... |511|
    ///       /                \
    ///  KERNEL_PDP          KERNEL_PDP_HIGH 
    ///                          \
    ///                      

    ArchVmAspace::ArchVmAspace(VirtAddr base, usize size, VmasFlags flags)
        : range_(base, size),
          flags_(flags)
    {}

    auto ArchVmAspace::init() -> Status
    {
        if (bool(VmasFlags::Kernel & flags_)) [[unlikely]] {
            auto const virt_pgd = reinterpret_cast<PhysAddr>(KERNEL_PGD);
            auto const phys_pgd = PhysMap::virt_to_phys(virt_pgd);
            auto status = page_table_.init(phys_pgd, virt_pgd, flags_);
            if (status != Status::Ok) {
                return status;
            }
        } else [[likely]] {
            auto status = page_table_.init(flags_);
            if (status != Status::Ok) {
                return status;
            }
        }

        return Status::Ok;
    }
}
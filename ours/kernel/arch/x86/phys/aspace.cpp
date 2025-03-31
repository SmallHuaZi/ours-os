#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <arch/system.hpp>
#include <arch/tlb.hpp>

#include <ustl/algorithms/copy.hpp>

namespace ours::phys {
    auto Aspace::arch_install() const -> void {
        // Create a shadow of CR4
        auto cr4 = arch::Cr4::read();

        // Disable global paging.
        cr4.set<cr4.Pge>(0);
        arch::Cr4::write(cr4);

        // Reload CR3 and flush all TLB.
        arch::Cr3::from_value(lower_pgd_).write();

        // Enable global paging.
        cr4.set<cr4.Pge>(1);
        arch::Cr4::write(cr4);
    }

    typedef Aspace::LowerPaging::PagingTraits Paging;

    auto arch_setup_aspace(Aspace &aspace) -> void {
        auto const mem = global_bootmem(); 
        DEBUG_ASSERT(mem, "`bootmem` must be setup before invoking `{}`", __func__);

        // Do our best to preserve the maximum amount of DMA memory. 
        mem->set_allocation_control(bootmem::AllocationControl::TopDown);

        // Restricting the root page table to the [0, 4GB) range facilitates the transition 
        // of APs from protected mode to long mode during boot.
        aspace.set_allocation_bounds(0, GB(4));
        aspace.init();

        // Cancel the allocation bounds of page table and create identity map of RAM under ARCH_PHYSMAP_SIZE.
        aspace.set_allocation_bounds(0, 0);
        create_identity_map_for_ram(aspace, ARCH_PHYSMAP_SIZE);

        aspace.install();
        while (1);
    }

} // namespace ours::phys
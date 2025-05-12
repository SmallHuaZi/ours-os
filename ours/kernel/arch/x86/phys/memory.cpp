#include <ours/arch/aspace_layout.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/acpi.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>

#include <acpi/numa.hpp>

namespace ours::phys {
    static bootmem::Region s_bootstrap_memory_regions[100];
    static bootmem::Region s_bootstrap_reserved_regions[100];

    BootMem g_bootmem {
        bootmem::RegionVector(&g_bootmem, s_bootstrap_memory_regions, std::size(s_bootstrap_memory_regions)),
        bootmem::RegionVector(&g_bootmem, s_bootstrap_reserved_regions, std::size(s_bootstrap_reserved_regions))
    };

    auto arch_init_memory(Aspace *aspace) -> void {
        auto mem = global_bootmem();

        // Reserve the first 1MB
        // mem->protect(0, MB(1));

        // Set bootmem to allocate always starting from the top address. 
        // As much as possible to reserve more DMA-able memory.
        mem->set_allocation_control(bootmem::AllocationControl::TopDown);

        if (aspace) {
            arch_setup_aspace(*aspace);
        }
    }

} // namespace ours::phys
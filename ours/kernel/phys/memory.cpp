#include <ours/phys/init.hpp>
#include <omitl/obi-view.hpp>

namespace ours::phys {
    auto obi_init_memory(PhysAddr obi) -> void {
    }

    /// We do not require any architecture to provide an implementation
    /// for this. Since 
    WEAK
    auto arch_detect_memory() -> void
    {}

    auto init_memory(PhysAddr obi, Aspace *aspace) -> void {
        auto const mem = global_bootmem();
        if (!global_bootmem()) {
            // 1. Look up OBI item of type OMIT_MEMCONFIG 
            //    If existing, to parse it and initialize `BootMem`
            obi_init_memory(obi);

            // Failed to parse the OBI item of type OMIT_MEMCONFIG. We will attempt 
            // to call the lowest-level architecture probe function.
            if (!global_bootmem()) {
                arch_detect_memory();
            }
        }
        mem->trim(PAGE_SIZE);

        // Reserve the image of kernel.phys, it was usually loaded above 1MB. 
        auto const phys_start = PhysAddr(kPhysLoadStart);
        auto const phys_end = PhysAddr(kPhysLoadEnd);
        mem->protect(phys_start, phys_end);

        arch_init_memory(aspace);
    }
}
#include <ours/phys/init.hpp>
#include <omitl/obi-view.hpp>

namespace ours::phys {
    extern char const kPhysLoadStart[] LINK_NAME("__executable_start");
    extern char const kPhysLoadEnd[] LINK_NAME("__executable_end");

    Aspace  *g_aspace;

    auto obi_init_memory(PhysAddr obi) -> void {
        auto obi_view = omitl::ObiView::from(reinterpret_cast<omitl::ObiHeader *>(obi));
        for (auto i = obi_view.begin(); i != obi_view.end(); ++i) {
            if (i->header->type != omitl::ObiType::Memory) {
                continue;
            }

            // TODO(SmallHuaZi): We should use `obi_view` to parse the memory region.
        }
    }

    /// We do not require any architecture to provide an implementation
    /// for this.
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
        // Reserve the image of kernel.phys, it was usually loaded above 1MB. 
        auto const phys_start = PhysAddr(kPhysLoadStart);
        auto const phys_end = PhysAddr(kPhysLoadEnd);
        mem->protect(phys_start, phys_end);

        if (aspace) {
            g_aspace = aspace;
        }

        arch_init_memory(aspace);

        // Is it necessary?
        mem->trim(PAGE_SIZE);

    }
}
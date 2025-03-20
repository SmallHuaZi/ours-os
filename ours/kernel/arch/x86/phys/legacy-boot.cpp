#include "legacy-boot.hpp"
#include <ours/phys/acpi.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>

#include <acpi/numa.hpp>

namespace ours::phys {
    LegacyBoot LegacyBoot::LEGACY_BOOT;

    static auto try_assign_node_for_bootmem(LegacyBoot const &legacyboot) -> void {
        auto acpi_parser = make_apic_parser(legacyboot.acpi_rsdp);
        if (!acpi_parser) {
            return;
        }

        auto status = acpi::enumerate_numa_region(acpi_parser.unwrap(), [](u32 numa_domain, acpi::NumaRegion const &region) {
            global_bootmem()->set_node(region.base, region.size, numa_domain);
            println("NUMA({}) region({:X}, {:X})", numa_domain, region.base, region.size);
        });
        if (status != Status::Ok) {
            println("No NUMA");
        }

        // Dump all retions in bootmem.
        auto bm = global_bootmem();
        bootmem::IterationContext context{bm, bootmem::RegionType::Normal};
        while (auto region = bm->iterate(context)) {
            println("Region({:X}, {:X}, {})", region->base, region->size, region->nid());
        }
    }

    WEAK
    auto arch_init_memory(Aspace *aspace) -> void {}

    auto init_memory(usize params, Aspace *aspace) -> void {
        LegacyBoot::get().init_memory(params);

        // For identity mapping in early stage.
        global_bootmem()->set_allocation_bounds(0, GB(1));

        try_assign_node_for_bootmem(LegacyBoot::get());

        if (aspace) {
            arch_init_memory(aspace);
        }
    }

    auto init_uart_console() -> void {}

} // namespace ours::phys
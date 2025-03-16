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
            println("NUMA region({}, {})", region.base, region.size);
        });
        if (status != Status::Ok) {
            println("No NUMA");
        }
    }

    WEAK
    auto arch_init_memory(Aspace *aspace) -> void {}

    auto init_memory(usize params, Aspace *aspace) -> void {
        LegacyBoot::get().init_memory(params, aspace);
        // For identity mapping in early stage.
        global_bootmem()->set_alloc_bounds(0, GB(1));

        try_assign_node_for_bootmem(LegacyBoot::get());

        if (aspace) {
            arch_init_memory(aspace);
        }
    }

    auto init_uart_console() -> void {}

} // namespace ours::phys
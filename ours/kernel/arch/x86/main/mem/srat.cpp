#include <ours/platform/acpi.hpp>

#include <ours/mem/early-mem.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/pmm.hpp>
#include <acpi/numa.hpp>

#include <logz4/log.hpp>
#include <ustl/mem/object.hpp>

namespace ours {
    static auto register_meminfo(acpi::IAcpiParser &acpi_parser) -> Status {
        log::trace("|{} | {:18} | {:18}", "Domain", "Base", "Size");
        auto status = acpi::enumerate_numa_region(acpi_parser, [] (u32 domain, acpi::NumaRegion const &region) {
            log::trace("|{:6} | 0x{:16X} | 0x{:16X}", domain, region.base, region.size);
            if (domain < MAX_NODES) {
                // Not all of machine are NUMA architecture. To support both NUMA and non-NUMA machine simultaneously,
                // we just register affinity infomation of memory about NUMA.
                mem::EarlyMem::set_node(region.base, region.size, domain);
            }
        });
        if (status != Status::Ok) {
            log::trace("No NUMA");
            return status;
        }

        // Dump all retions in bootmem.
        // auto bm = global_bootmem();
        // bootmem::IterationContext context{bm, bootmem::RegionType::Normal};
        // while (auto region = bm->iterate(context)) {
        //     println("|{} | 0x{:16X} | 0x{:16X}", region->base, region->size, region->nid());
        // }

        return Status::Ok;
    }

    static auto register_nodedis(acpi::IAcpiParser &acpi_parser) -> Status {
        auto const num_domain = acpi::count_numa_domain(acpi_parser);
        auto numa_distance = mem::EarlyMem::allocate<u8>(num_domain * num_domain, alignof(usize));
        if (!numa_distance) {
            return Status::OutOfMem;
        }

        auto status = acpi::enumerate_numa_domain_distance(acpi_parser,
            [numa_distance, num_domain] (u32 from, u32 to, u8 dis) {
                numa_distance[from * num_domain + to] = dis;
            }
        );

        if (status != Status::Ok) {
            log::info("No NUMA");
            mem::EarlyMem::deallocate(numa_distance, num_domain);
            return status;
        }

        return Status::Ok;
    }

    auto acpi_init_numa() -> Status {
        auto parser = get_acpi_parser();
        DEBUG_ASSERT(parser, "");
        if (!parser) {
            return Status::NotFound;
        }

        Status status = register_meminfo(*parser);
        if (status != Status::Ok) {
            return status;
        }

        status = register_nodedis(*parser);
        if (status != Status::Ok) {
            return status;
        }

        return Status::Ok;
    }

} // namespace ours
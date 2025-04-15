#include <ours/platform/acpi.hpp>

#include <ours/mem/early-mem.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/mem/pmm.hpp>
#include <acpi/numa.hpp>

#include <logz4/log.hpp>
#include <ustl/mem/object.hpp>

namespace ours {
    static auto print_numa_info() -> void {
        auto parser = get_acpi_parser();
        log::trace("NUMA information from ACPI-SRAT");
        log::trace("|{} | {:18} | {:18}", "Domain", "Base", "Size");
        auto status = acpi::enumerate_numa_region(*parser, 
            [] (u32 domain, acpi::NumaRegion const &region) {
            log::trace("|{:6} | 0x{:16X} | 0x{:16X}", domain, region.base, region.size);
        });

        if (status != Status::Ok) {
            log::trace("No NUMA");
        }
    }

    static auto register_meminfo(acpi::IAcpiParser &parser) -> Status {
        auto status = acpi::enumerate_numa_region(parser, 
            [] (u32 domain, acpi::NumaRegion const &region) {
            log::trace("|{:6} | 0x{:16X} | 0x{:16X}", domain, region.base, region.size);
            if (domain < MAX_NODES) {
                // Not all of machine are NUMA architecture. To support both NUMA and non-NUMA machine simultaneously,
                // we just register affinity infomation of memory about NUMA.
                mem::EarlyMem::set_node(region.base, region.size, domain);
                mem::set_node_state(domain, mem::NodeStates::Possible, true);
            }
        });
        if (status != Status::Ok) {
            return status;
        }

        return Status::Ok;
    }

    static auto register_nodedis(acpi::IAcpiParser &parser) -> Status {
        auto status = acpi::enumerate_numa_domain_distance(parser,
            [] (u32 from, u32 to, u8 dis) {
                mem::PmNode::set_distance(from, to, dis);
            }
        );

        if (status != Status::Ok) {
            log::info("No NUMA");
            for (auto i = 0; i < MAX_NODES; ++i) {
                for (auto j = 0; j < MAX_NODES; ++j) {
                    mem::PmNode::set_distance(i, j, 0);
                }
            }
            return status;
        }

        return Status::Ok;
    }

    /// FIXME(SmallHuaZi) Temporary mapping to forward development in CpuLocal.
    /// It should be the content of topology.
    static auto bind_node_to_cpu(acpi::IAcpiParser &parser) -> Status {
        INIT_DATA
        static usize apicids[MAX_CPU_NUM];

        INIT_DATA
        static usize cpus[MAX_CPU_NUM];

        usize count = 0;
        return acpi::enumerate_cpu_numa_pairs(parser, [&] (usize apicid, usize nodeid) {
            apicids[count] = apicid;
            cpus[count] = count;
            cpu_possible_mask().set(count);
            mem::bind_cpu_to_node(count++, nodeid);
        });
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

        // For debug
        print_numa_info();

        mem::EarlyMem::dump();

        status = register_nodedis(*parser);
        if (status != Status::Ok) {
            return status;
        }

        status = bind_node_to_cpu(*parser);
        if (status != Status::Ok) {
            return status;
        }

        return Status::Ok;
    }

} // namespace ours
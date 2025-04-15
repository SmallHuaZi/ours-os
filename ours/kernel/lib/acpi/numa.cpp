#include <acpi/numa.hpp>
#include <ustl/io/binary_reader.hpp>
#include <ustl/traits/is_same.hpp>

using ours::Status;

namespace acpi {
    auto enumerate_numa_region(AcpiSrat const &srat, NumaRegionCommitFn const &commit) -> Status {
        auto reader = ustl::io::BinaryReader::from_payload_of_struct(&srat);
        while (reader) {
            auto header = reader.read<AcpiEntryHeader>();
            if (!header) {
                return Status::InternalError;
            }
            if (header->type != AcpiSratTypeMemoryAffinity) {
                continue;
            }

            auto memory = downcast<AcpiSratMemoryAffinityEntry>(header);
            if (!memory) {
                return Status::InternalError;
            }
            if (!(memory->flags & AcpiSratFlagEnabled)) {
                continue;
            }

            NumaRegion region;
            region.base = memory->base_address_low | (u64(memory->base_address_high) << 32);
            region.size = memory->length_low | (u64(memory->length_high) << 32);
            commit(memory->proximity_domain, region);
        }

        return Status::Ok;
    }

    /// Parse SRAT to acquire the map between APICID and proximity domain(namely NUMA domain).
    auto enumerate_cpu_numa_pairs(AcpiSrat const &srat, CpuNumaPairsCommitFn const &commit) -> ours::Status {
        auto reader = ustl::io::BinaryReader::from_payload_of_struct(&srat);
        while (reader) {
            auto header = reader.read<AcpiEntryHeader>();
            if (!header) {
                return Status::InternalError;
            }

            if (header->type == AcpiSratTypeProcessorAffinity) {
                auto processor = downcast<AcpiSratProcessorAffinityEntry>(header);
                if (!processor) {
                    return Status::InternalError;
                }
                if (!(processor->flags & AcpiSratFlagEnabled)) {
                    return Status::InternalError;
                }
                commit(processor->apic_id, processor->proximity_domain());
            } else if (header->type == AcpiSratTypeProcessorX2ApicAffinity) {
                auto processor = downcast<AcpiSratProcessorX2ApicAffinityEntry>(header);
                if (!processor) {
                    return Status::InternalError;
                }
                if (!(processor->flags & AcpiSratFlagEnabled)) {
                    return Status::InternalError;
                }
                commit(processor->x2apic_id, processor->proximity_domain);
            }
        }

        return Status::Ok;
    }

    /// Parse SLIT to acquire the distance map among all proximity domains.
    auto enumerate_numa_domain_distance(AcpiSlit const &slit, NumaDistanceCommitFn const &commit) -> Status {
        auto const num_domain = slit.locality_count;
        for (u64 i = 0; i < num_domain; ++i) {
            for (u64 j = 0; j < num_domain; ++j) {
                commit(i, j, slit.entry[i * num_domain + j]);
            }
        }

        return Status::Ok;
    }

} // namespace acpi
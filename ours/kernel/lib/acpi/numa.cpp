#include <acpi/numa.hpp>
#include <ustl/io/binary_reader.hpp>
#include <ustl/traits/is_same.hpp>

using ours::Status;

namespace acpi {
    auto enumerate_numa_region(AcpiParser const &parser, NumaRegionCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSignature{"SRAT"});
        ustl::io::BinaryReader reader(srat, srat->size());
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

    auto enumerate_cpu_numa_pairs(AcpiParser const &parser, CpuNumaPairsCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSignature{"SRAT"});
        ustl::io::BinaryReader reader(srat, srat->size());
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

} // namespace acpi
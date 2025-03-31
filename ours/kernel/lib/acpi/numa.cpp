#include <acpi/numa.hpp>
#include <ustl/io/binary_reader.hpp>
#include <ustl/traits/is_same.hpp>

using ours::Status;

namespace acpi {
    auto count_numa_domain(IAcpiParser const &parser) -> usize {
        auto const header = get_table_by_signature(parser, AcpiSignature{"SLIT"});
        if (!header) {
            return 0;
        }

        auto slit = reinterpret_cast<AcpiSlit const *>(header);
        return slit->locality_count;
    }

    auto enumerate_numa_region(IAcpiParser const &parser, NumaRegionCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSignature{"SRAT"});
        if (!srat) {
            return Status::NotFound;
        }

        ustl::io::BinaryReader reader(srat, srat->size());
        reader.skip_bytes(sizeof(AcpiSrat));
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

    auto enumerate_cpu_numa_pairs(IAcpiParser const &parser, CpuNumaPairsCommitFn const &commit) -> Status {
        auto const srat = get_table_by_signature(parser, AcpiSignature{"SRAT"});
        if (!srat) {
            return Status::NotFound;
        }
        ustl::io::BinaryReader reader(srat, srat->size());
        reader.skip_bytes(sizeof(AcpiSrat));
        while (reader) {
            auto header = reader.read<AcpiEntryHeader>();
            if (!header) {
                return Status::InternalError;
            }

            reader.skip_bytes(header->size() - sizeof(AcpiEntryHeader));
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

    auto enumerate_numa_domain_distance(IAcpiParser const &parser, NumaDistanceCommitFn const &commit) -> Status
    {
        auto const slit_header = get_table_by_signature(parser, AcpiSignature{"SLIT"});
        if (!slit_header)  {
            return Status::NotFound;
        }

        auto slit = reinterpret_cast<AcpiSlit const *>(slit_header);
        auto const num_domain = slit->locality_count;
        for (u64 i = 0; i < num_domain; ++i) {
            for (u64 j = 0; j < num_domain; ++j) {
                commit(i, j, slit->entry[i * num_domain + j]);
            }
        }

        return Status::Ok;
    }

} // namespace acpi
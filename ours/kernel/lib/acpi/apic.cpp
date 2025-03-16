#include <acpi/apic.hpp>
#include <acpi/details/madt.hpp>
#include <ours/status.hpp>

#include <ustl/function/fn.hpp>
#include <ustl/io/binary_reader.hpp>

namespace acpi {
    using ours::Status;

    template <typename Entry, typename Visitor>
    auto enumerate_madt_entry_for_type(AcpiParser const &parser, u8 const type, Visitor visitor) -> Status {
        auto const madt = get_table_by_signature(parser, AcpiSignature("SART"));
        ustl::io::BinaryReader reader(madt, madt->size());
        while (reader) {
            auto header = reader.read<AcpiEntryHeader>();
            if (!header) {
                return Status::InternalError;
            } else if (header->type != type) {
                continue;
            }

            auto entry = downcast<Entry>(header);
            if (!entry) {
                return Status::InternalError;
            }

            visitor(*entry);
        }

        return Status::Ok;
    }

    auto enumerate_io_apics(AcpiParser const &parser, IoApicCommitFn const &commit) -> Status {
        return enumerate_madt_entry_for_type<AcpiMadtIoApicEntry>(parser, AcpiMadtTypeIoApic, commit);
    }

    auto enumerate_local_apics(AcpiParser const &parser, LocalApicCommitFn const &commit) -> Status {
        // clang-format off
        return enumerate_madt_entry_for_type<AcpiMadtLocalApicEntry>(
            parser, AcpiMadtTypeLocalApic,
            [&](AcpiMadtLocalApicEntry const &entry) {
            if (entry.flags & AcpiMadtFlagEnabled) {
                return commit(entry);
            }
        });
        // clang-format on
    }

    auto enumerate_io_apic_isa_overrides(AcpiParser const &parser, IntrOverridesCommitFn const &commit) -> Status {
        return enumerate_madt_entry_for_type<AcpiMadtInterruptOverrideEntry>(parser, AcpiMadtTypeInterruptOverride, commit);
    }

} // namespace acpi

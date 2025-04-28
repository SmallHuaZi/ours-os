#include <acpi/apic.hpp>
#include <acpi/details/madt.hpp>
#include <ours/status.hpp>

#include <ustl/function/fn.hpp>
#include <ustl/io/binary_reader.hpp>

namespace acpi {
    using ours::Status;

    template <typename Entry, typename Visitor>
    FORCE_INLINE
    auto enumerate_madt_entry_for_type(AcpiMadt const &madt, u8 const type, Visitor visitor) -> Status {
        auto reader = ustl::io::BinaryReader::from_payload_of_struct(&madt);
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

    auto enumerate_io_apics(AcpiMadt const &madt, IoApicCommitFn const &commit) -> Status {
        return enumerate_madt_entry_for_type<AcpiMadtIoApicEntry>(madt, AcpiMadtTypeIoApic, commit);
    }

    auto enumerate_local_apics(AcpiMadt const &madt, LocalApicCommitFn const &commit) -> Status {
        // clang-format off
        return enumerate_madt_entry_for_type<AcpiMadtLocalApicEntry>(
            madt, AcpiMadtTypeLocalApic,
            [&](AcpiMadtLocalApicEntry const &entry) {
            if (entry.flags & AcpiMadtFlagEnabled) {
                return commit(entry);
            }
        });
        // clang-format on
    }

    auto enumerate_io_apic_isa_overrides(AcpiMadt const &madt, IntrOverridesCommitFn const &commit) -> Status {
        return enumerate_madt_entry_for_type<AcpiMadtInterruptOverrideEntry>(madt, AcpiMadtTypeInterruptOverride, commit);
    }

} // namespace acpi

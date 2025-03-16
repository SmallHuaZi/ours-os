/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///

#ifndef ACPI_HEADER_HPP
#define ACPI_HEADER_HPP 1

#include <ours/config.hpp>
#include <ours/types.hpp>
#include <ours/status.hpp>

#include <ustl/traits/is_same.hpp>

namespace acpi {
    typedef ours::Status    Status;

    struct PACKED AcpiSignature {
        // Create an AcpiSignature from a C-style string.
        CXX11_CONSTEXPR
        AcpiSignature() = default;

        CXX11_CONSTEXPR
        explicit AcpiSignature(const char name[4])
            : value(name[0] << 24 | name[1] << 16 | name[2] << 8 | name[3]) {}

        // Operators.
        CXX11_CONSTEXPR
        friend bool operator==(AcpiSignature const &left, AcpiSignature const &right) {
            return left.value == right.value;
        }

        CXX11_CONSTEXPR
        friend bool operator!=(AcpiSignature const &left, AcpiSignature const &right) {
            return left.value != right.value;
        }

        // Value, in big-endian format to match the in-memory representation.
        //
        // For example, on little-endian systems the signature "1234" will have a value
        // 0x34'33'32'31, with bytes reversed.
        u32 value;
    };

    /// System description table header.
    struct PACKED AcpiTableHeader {
        CXX11_CONSTEXPR
        auto size() const -> usize {
            return length;
        }

        AcpiSignature sign;
        u32 length;
        u8 revision;
        u8 checksum;
        char oem_id[6];
        char oem_tableid[8];
        u32 oem_revision;
        u32 creator_id;
        u32 creator_revision;
    };

    struct PACKED AcpiEntryHeader {
        u8 type;
        u8 length;

        CXX11_CONSTEXPR
        auto size() const -> u8 {
            return length;
        }
    };

    template <typename Dest, typename Src>
    auto downcast(const Src* src) -> Dest const * {
        static_assert(offsetof(Dest, header) == 0,
                      "Expected field |header| to be first field in struct.");
        static_assert(ustl::traits::IsSameV<decltype(Dest::header), Src>,
                      "Expected |Dest::header| type to match |Src|.");
        if (src->size() < sizeof(Dest)) {
            return nullptr;
        }
        return reinterpret_cast<const Dest*>(src);
    }

} // namespace acpi

#endif // #ifndef ACPI_HEADER_HPP
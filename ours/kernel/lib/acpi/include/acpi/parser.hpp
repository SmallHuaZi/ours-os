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
#ifndef ACPI_PARSER_HPP
#define ACPI_PARSER_HPP 1

#include <acpi/details/header.hpp>
#include <acpi/details/rdst.hpp>

#include <ktl/result.hpp>

namespace acpi {
    using ours::PhysAddr;

    struct IPhysToVirt {
        virtual ~IPhysToVirt() = default;
        virtual auto phys_to_virt(PhysAddr addr, usize size) -> ktl::Result<ai_virt void const *> = 0;
    };

    struct IAcpiParser {
        virtual ~IAcpiParser() = default;
        virtual auto num_tables() const -> usize = 0;
        virtual auto get_table_at_index(usize index) const -> ktl::Result<AcpiTableHeader const *> = 0;
    };

    struct AcpiParser: public IAcpiParser {
        static auto from_rsdp(IPhysToVirt *mapper, PhysAddr rsdp) -> ktl::Result<AcpiParser>;

        auto num_tables() const -> usize override {
            return num_tables_;
        }

        auto get_table_at_index(usize index) const -> ktl::Result<AcpiTableHeader const *> override;
        auto get_table_phys_addr(usize index) const -> PhysAddr;

      private:
        FORCE_INLINE CXX11_CONSTEXPR
        AcpiParser(IPhysToVirt *p2v, PhysAddr rsdp_addr, AcpiXsdt const *xsdt, AcpiRsdt const *rsdt,
                   PhysAddr root_table_paddr, usize num_tables)
            : p2v_(p2v),
              rsdp_paddr_(rsdp_addr),
              rsdt_(rsdt),
              xsdt_(xsdt),
              root_table_paddr_(root_table_paddr),
              num_tables_(num_tables) {}

        IPhysToVirt *p2v_;
        AcpiRsdt const *rsdt_;
        AcpiXsdt const *xsdt_;
        PhysAddr rsdp_paddr_;
        PhysAddr root_table_paddr_;
        usize num_tables_;
    };

    auto calc_checksum(void const *buffer, usize len) -> u8;

    FORCE_INLINE CXX11_CONSTEXPR 
    auto validate_checksum(void const *buffer, usize len) -> bool {
        return 0 == calc_checksum(buffer, len);
    }

    template <typename Table>
    FORCE_INLINE 
    auto validate_table(ai_virt Table *table) -> bool {
        if (table->header.sign != Table::kSignature) {
            return false;
        }
        if (!validate_checksum(table, table->header.size())) {
            return false;
        }

        return true;
    }

    auto get_table_by_signature(IAcpiParser const &, AcpiSignature) -> AcpiTableHeader const *;

    template <typename Table>
    auto get_table_by_type(IAcpiParser const &parser) -> Table const * {
        return reinterpret_cast<Table const *>(get_table_by_signature(parser, Table::kSignature));
    }

} // namespace acpi

#endif // #ifndef ACPI_PARSER_HPP
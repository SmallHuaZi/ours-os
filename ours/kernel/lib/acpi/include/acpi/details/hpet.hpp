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
#ifndef ACPI_DETAILS_HPET_HPP
#define ACPI_DETAILS_HPET_HPP 1

#include <ours/assert.hpp>
#include <acpi/details/header.hpp>
#include <ustl/bitfields.hpp>

namespace acpi {
    struct PACKED AcpiHpet {
        AcpiTableHeader header;
        u32 id;
        AcpiGenericAddress address;
        u8 hpet_number;
        u16 minimum_tick;
        u8 page_protection;

        CXX11_CONSTEXPR
        static auto kSignature = AcpiSignature("HPET");
    };

    struct PACKED HpetTimerRegs {
        template <usize Id, usize Bits, ustl::util::FixedString Name>
        using Field = ustl::bitfields::Field<
            ustl::bitfields::Id<Id>, 
            ustl::bitfields::Bits<Bits>,
            ustl::bitfields::Name<Name>>;

        struct ConfigAndCapRegs {
            typedef ConfigAndCapRegs    Self;
            enum FieldId {
                kReserved0,
                kIntrTypeConfig,
                kIntrEnableConfig,
                kTimerTypeConfig,
                kPeriodicCap,
                kSizeCap,
                kValSetConfig,
                kReserved1,
                k32ModeConfig,
                kIntrRouteConfig,
                kTnFSBEnConfig,
                kTnFSBIntrDelCap,
                kReserved2,
                kTnIntrRouteCap
            };

            FORCE_INLINE
            auto mask(bool mask = true) volatile -> Self volatile & {
                inner.set<kIntrEnableConfig>(!mask);
                return *this;
            }

            FORCE_INLINE
            auto irq() const volatile -> usize {
                return inner.get<kIntrRouteConfig>();
            }

            FORCE_INLINE
            auto configure_irq(usize irqnum) volatile -> Self volatile & {
                inner.set<kIntrRouteConfig>(irqnum);
                return *this;
            }

            FORCE_INLINE
            auto has_periodic() const volatile -> bool {
                return inner.get<kPeriodicCap>();
            }

            FORCE_INLINE
            auto enable_periodic() volatile -> Self volatile & {
                DEBUG_ASSERT(has_periodic());
                inner.set<kTimerTypeConfig>(1);
                inner.set<kValSetConfig>(1);
                return *this;
            }

            enum TriggerMode {
                Edge,
                Level,
            };

            FORCE_INLINE
            auto trigger_mode() volatile -> TriggerMode {
                return (TriggerMode)inner.get<kIntrTypeConfig>();
            }

            volatile ustl::BitFields<
                Field<kReserved0, 1, "Reserved">,
                // 0-edge-triggered interrupts.
                // 1-level-triggered interrupts. 
                Field<kIntrTypeConfig, 1, "IntrTypeConfig">,
                // 0-this timer will still set Tn_INT_STS.
                // 1-enables triggering of interrupts. 
                Field<kIntrEnableConfig, 1, "IntrEnableConfig">,
                // 1-enables periodic type.
                // 0-enables oneshot type.
                Field<kTimerTypeConfig, 1, "TimerTypeConfig">,
                // Read-only.
                // 1-this timer supports periodic mode.
                Field<kPeriodicCap, 1, "PeriodicCap">,
                // 1-supports 64-bit.
                // 0-supports 32-bit.
                Field<kSizeCap, 1, "SizeCap">,
                // Be used to allow software to directly set periodic timer's accumulator.
                Field<kValSetConfig, 1, "ValSetConfig">,
                Field<kReserved1, 1, "Reserved">,
                // Force to enable 32-bit timer.
                Field<k32ModeConfig, 1, "32ModeConfig">,
                // This field indicates I/O APIC routing. Allowed values can be determined 
                // using Tn_INT_ROUTE_CAP. If an illegal value is written, then value read 
                // back from this field will not match the written value.
                Field<kIntrRouteConfig, 5, "IntrRouteConfig">,
                // 1-this timer will use FSB interrupt mapping.
                Field<kTnFSBEnConfig, 1, "TnFSBEnConfig">,
                // 1-this timer supports FSB interrupt mapping.
                Field<kTnFSBIntrDelCap, 1, "TnFSBIntrDelCap">,
                Field<kReserved2, 16, "Reserved">,
                // Timer n Interrupt Routing Capability. If bit X is set in this field, 
                // it means that this timer can be mapped to IRQX line of I/O APIC.
                Field<kTnIntrRouteCap, 32, "TnIntrRouteCap">
            > inner;           
        };

        volatile ConfigAndCapRegs conf_caps;
        volatile u64 comparator_value;
        volatile u64 fsb_int_route;
        u8 _reserved[8];
    };

    struct PACKED HpetRegs {
        template <usize Id, usize Bits, ustl::util::FixedString Name>
        using Field = ustl::bitfields::Field<
            ustl::bitfields::Id<Id>, 
            ustl::bitfields::Bits<Bits>,
            ustl::bitfields::Name<Name>>;
        
        struct PACKED HpetGeneralCapAndIdRegs {
            typedef HpetGeneralCapAndIdRegs Self;
            auto revid() const volatile -> u16 {
                return inner.get<0>();
            }

            auto num_timers() const volatile-> u16 {
                return inner.get<1>() + 1;
            }

            auto support_64bit() const volatile-> bool {
                return inner.get<2>();
            }

            auto vendor_id() const volatile-> u16 {
                return inner.get<5>();
            }

            auto period() const volatile-> u16 {
                return inner.get<6>();
            }

            auto set_period(u32 ns) volatile -> Self volatile & {
                inner.set<6>(ns);
                return *this;
            }

            volatile ustl::BitFields<
                Field<0, 8, "RevId">,
                Field<1, 5, "NumTimers-1">,
                Field<2, 1, "CounterSize">, // If one bit is set, the counter is 64 bits
                Field<3, 1, "Reserved">,
                Field<4, 1, "LegacyRoute">,
                Field<5, 16, "VendorId">,
                Field<6, 32, "Period">
            > inner;
        };
        static_assert(sizeof(HpetGeneralCapAndIdRegs) == 8);

        struct PACKED HpetGeneralConfigRegs {
            typedef HpetGeneralConfigRegs   Self;
            auto disable_cnf() volatile -> void {
                inner.set<0>(0);
            }

            auto enable_cnf() volatile -> void {
                inner.set<0>(1);
            }

            auto is_legacy_route() const volatile -> bool {
                return inner.get<1>();
            }

            auto enable_legacy_route(bool mask) volatile -> Self volatile & {
                inner.set<1>(mask);
                return *this;
            }

            volatile ustl::BitFields<
                Field<0, 1, "EnableCNF">,
                Field<1, 1, "LegacyRoute">,
                Field<2, 60, "Reserved">
            > inner;
        };
        static_assert(sizeof(HpetGeneralCapAndIdRegs) == 8);

        volatile HpetGeneralCapAndIdRegs general_caps;
        u8 _reserved0[8];
        volatile HpetGeneralConfigRegs general_config;
        u8 _reserved1[8];
        volatile u64 general_intr_status;
        u8 _reserved2[0xf0 - 0x28];
        volatile u64 main_counter_value;
        u8 _reserved3[8];
        HpetTimerRegs timers[];
    };

} // namespace acpi

#endif // #ifndef ACPI_DETAILS_HPET_HPP
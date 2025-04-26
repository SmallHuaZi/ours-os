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

#ifndef ARCH_X86_APIC_XAPIC_HPP
#define ARCH_X86_APIC_XAPIC_HPP 1

#include <arch/intr_disable_guard.hpp>
#include <arch/types.hpp>
#include <arch/x86/apic/common.hpp>
#include <arch/x86/msr.hpp>

namespace arch {
    enum class XApicRegType : u32 {
        Id = 0x020,
        Version = 0x030,
        Tpr = 0x080,
        Ppr = 0x0A0,
        Eoi = 0x0B0,
        Ldr = 0x0D0,
        Svr = 0x0F0,
        Isr0 = 0x100,
        Isr1 = 0x110,
        Isr2 = 0x120,
        Isr3 = 0x130,
        Isr4 = 0x140,
        Isr5 = 0x150,
        Isr6 = 0x160,
        Isr7 = 0x170,
        Tmr0 = 0x180,
        Tmr1 = 0x190,
        Tmr2 = 0x1A0,
        Tmr3 = 0x1B0,
        Tmr4 = 0x1C0,
        Tmr5 = 0x1D0,
        Tmr6 = 0x1E0,
        Tmr7 = 0x1F0,
        Irr0 = 0x200,
        Irr1 = 0x210,
        Irr2 = 0x220,
        Irr3 = 0x230,
        Irr4 = 0x240,
        Irr5 = 0x250,
        Irr6 = 0x260,
        Irr7 = 0x270,
        Esr = 0x280,
        LvtCmci = 0x2F0,
        Icr0 = 0x300,
        Icr1 = 0x310,
        LvtTimer = 0x320,
        LvtThermal = 0x330,
        LvtPmi = 0x340,
        LvtLint0 = 0x350,
        LvtLint1 = 0x360,
        LvtError = 0x370,
        TimerInitCount = 0x380,
        TimerCurrentCount = 0x390,
        TimerDivConf = 0x3E0,
    };

    /// The Intel Software Developer's Manual, however states that,
    /// once you have disabled the local APIC through IA32_APIC_BASE
    /// you can't enable it anymore until a complete reset. The I/O APIC
    /// can also be configured to run in legacy mode so that it emulates
    /// an 8259 device.
    struct XApic {
        XApic(u32 *mmio_base)
            : mmio_virt_(mmio_base),
              id_(read_reg(XApicRegType::Id)),
              logic_id_(read_reg(XApicRegType::Ldr)),
              version_(read_reg(XApicRegType::Version))
        {}

        FORCE_INLINE
        auto id() const -> u32 {
            return id_;
        }

        FORCE_INLINE
        auto logical_id() const -> u32 {
            return logic_id_;
        }

        FORCE_INLINE
        auto version() const -> u32 {
            return version_;
        }

        FORCE_INLINE
        auto enable_tsc(u32 vector) -> void {
            auto lvt = read_reg(XApicRegType::LvtTimer);
            lvt &= 0xff;
            lvt |= vector;
            lvt &= ~BIT(16);
            lvt &= ~BIT(17);
            lvt |= BIT(18);
            write_reg(XApicRegType::LvtTimer, lvt);
        }

        FORCE_INLINE
        auto set_tsc_deadline(u64 deadline) -> bool {
            // Now i am not sure that if to set a deadline was constrained.
            return false;
        }

        FORCE_INLINE
        auto send_init_ipi() -> void {
            IpiRequest request;
            request.set_dest_mode(ApicDestinationMode::Physical)
                .set_delivery_mode(ApicDeliveryMode::Init)
                .set_delivery_status(ApicDeliveryStatus::Idle)
                .set_level(ApicIpiLevel::Assert)
                .set_trigger_mode(ApicTriggerMode::Level);
            broadcast_ipi_without_self(request);
        }

        FORCE_INLINE
        auto send_startup_ipi() -> void {
            IpiRequest request;
            request.set_dest_mode(ApicDestinationMode::Physical)
                .set_delivery_mode(ApicDeliveryMode::Startup)
                .set_delivery_status(ApicDeliveryStatus::Idle)
                .set_level(ApicIpiLevel::Assert)
                .set_trigger_mode(ApicTriggerMode::Edge);
            broadcast_ipi_without_self(request);
        }

        FORCE_INLINE
        auto send_eoi() -> void {
            write_reg(XApicRegType::Eoi, 0);
        }

        FORCE_INLINE
        auto send_ipi(u32 const apic_id, IpiRequest request) -> void {
            IntrDisableGuard guard;
            write_reg(XApicRegType::Esr, 0);
            write_reg(XApicRegType::Icr1, request.to_icr1());
            write_reg(XApicRegType::Icr0, request.to_icr0());
            wait_for_ipi();
        }

        FORCE_INLINE
        auto send_ipi_self(IpiRequest request) -> void {
            request.set_target(ApicIpiTarget::Self);
            send_ipi(0, request);
        }

        FORCE_INLINE
        auto broadcast_ipi(IpiRequest request) -> void {
            request.set_target(ApicIpiTarget::All);
            send_ipi(0, request);
        }

        FORCE_INLINE
        auto broadcast_ipi_without_self(IpiRequest request) -> void {
            request.set_target(ApicIpiTarget::AllButSelf);
            send_ipi(0, request);
        }

      private:
        FORCE_INLINE
        auto read_reg(XApicRegType type) -> u32 {
            return *get_reg_addr(type);
        }

        FORCE_INLINE
        auto write_reg(XApicRegType type, u32 val) -> void {
            *get_reg_addr(type) = val;
        }

        FORCE_INLINE
        auto get_reg_addr(XApicRegType type) const -> u32 volatile * {
            return (u32 volatile *)((u8 *)mmio_virt_ + u32(type));
        }

        FORCE_INLINE
        auto wait_for_ipi() -> void {
            CXX11_CONSTEXPR
            usize const delivery_pending = BIT(12);

            while (1) {
                if (!(read_reg(XApicRegType::Icr0) & delivery_pending)) {
                    break;
                } else if (read_reg(XApicRegType::Esr) > 0) {
                    break;
                }
            }
        }

        ai_virt u32 *mmio_virt_;
        u8 id_;
        u8 logic_id_;
        u8 version_;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_APIC_XAPIC_HPP
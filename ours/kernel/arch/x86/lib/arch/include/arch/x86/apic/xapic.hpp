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

#include <ustl/traits/enable_if.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_convertible.hpp>

namespace arch {
    enum class XApicRegType : u32 {
        Id = 0x020,                ///< ID Register (APIC ID)
        Version = 0x030,           ///< Version Register
        Tpr = 0x080,               ///< Task Priority Register (TPR)
        Ppr = 0x0A0,               ///< Processor Priority Register (PPR)
        Eoi = 0x0B0,               ///< End Of Interrupt Register (EOI)
        Ldr = 0x0D0,               ///< Logical Destination Register (LDR)
        Svr = 0x0F0,               ///< Spurious Interrupt Vector Register (SVR)
        Isr0 = 0x100,              ///< In-Service Register 0 (ISR0)
        Isr1 = 0x110,              ///< In-Service Register 1 (ISR1)
        Isr2 = 0x120,              ///< In-Service Register 2 (ISR2)
        Isr3 = 0x130,              ///< In-Service Register 3 (ISR3)
        Isr4 = 0x140,              ///< In-Service Register 4 (ISR4)
        Isr5 = 0x150,              ///< In-Service Register 5 (ISR5)
        Isr6 = 0x160,              ///< In-Service Register 6 (ISR6)
        Isr7 = 0x170,              ///< In-Service Register 7 (ISR7)
        Tmr0 = 0x180,              ///< Timer Register 0 (TMR0)
        Tmr1 = 0x190,              ///< Timer Register 1 (TMR1)
        Tmr2 = 0x1A0,              ///< Timer Register 2 (TMR2)
        Tmr3 = 0x1B0,              ///< Timer Register 3 (TMR3)
        Tmr4 = 0x1C0,              ///< Timer Register 4 (TMR4)
        Tmr5 = 0x1D0,              ///< Timer Register 5 (TMR5)
        Tmr6 = 0x1E0,              ///< Timer Register 6 (TMR6)
        Tmr7 = 0x1F0,              ///< Timer Register 7 (TMR7)
        Irr0 = 0x200,              ///< Interrupt Request Register 0 (IRR0)
        Irr1 = 0x210,              ///< Interrupt Request Register 1 (IRR1)
        Irr2 = 0x220,              ///< Interrupt Request Register 2 (IRR2)
        Irr3 = 0x230,              ///< Interrupt Request Register 3 (IRR3)
        Irr4 = 0x240,              ///< Interrupt Request Register 4 (IRR4)
        Irr5 = 0x250,              ///< Interrupt Request Register 5 (IRR5)
        Irr6 = 0x260,              ///< Interrupt Request Register 6 (IRR6)
        Irr7 = 0x270,              ///< Interrupt Request Register 7 (IRR7)
        Esr = 0x280,               ///< Error Status Register (ESR)
        LvtCmci = 0x2F0,           ///< Local Vector Table Register for Machine Check Architecture Interrupts (LVT CMCI)
        Icr0 = 0x300,              ///< Interrupt Command Register 0 (ICR0)
        Icr1 = 0x310,              ///< Interrupt Command Register 1 (ICR1)
        LvtTimer = 0x320,          ///< Local Vector Table Register for Timer Interrupt (LVT Timer)
        LvtThermal = 0x330,        ///< Local Vector Table Register for Thermal Interrupt (LVT Thermal)
        LvtPmi = 0x340,            ///< Local Vector Table Register for Performance Monitoring Interrupt (LVT PMI)
        LvtLogicalInt0 = 0x350,    ///< Local Vector Table Register for Logical Interrupt 0 (LVT LINT0)
        LvtLogicalInt1 = 0x360,    ///< Local Vector Table Register for Logical Interrupt 1 (LVT LINT1)
        LvtError = 0x370,          ///< Local Vector Table Register for Error Interrupt (LVT Error)
        TimerInitCount = 0x380,    ///< Timer Initial Count Register
        TimerCurrentCount = 0x390, ///< Timer Current Count Register
        TimerDivConf = 0x3E0,      ///< Timer Division Configuration Register
    };

    CXX11_CONSTEXPR
    static auto const kInvalidApicId = 0xffffffff;

    CXX11_CONSTEXPR
    static auto const kApicPhysBase = 0xfee00000;

    CXX11_CONSTEXPR
    static auto const kIa32ApicBaseBsp = BIT(8);

    CXX11_CONSTEXPR
    static auto const kIa32ApicBaseX2ApicEnable = BIT(10);

    CXX11_CONSTEXPR
    static auto const kIa32ApicBaseXapicEnable = BIT(11);

    /// The Intel Software Developer's Manual, however states that,
    /// once you have disabled the local APIC through IA32_APIC_BASE
    /// you can't enable it anymore until a complete reset. The I/O APIC
    /// can also be configured to run in legacy mode so that it emulates
    /// an 8259 device.
    struct XApic {
        FORCE_INLINE
        auto init(u32 *mmio_base) -> void {
            mmio_virt_ = mmio_base;
            id_ = read_reg(XApicRegType::Id);
            logic_id_ = read_reg(XApicRegType::Ldr);
            version_ = read_reg(XApicRegType::Version);
        }

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

        enum class TscMode {
            OneShot,
            Periodic,
            Deadline,
        };

        template <typename Vector>
        FORCE_INLINE
        auto enable_tsc(Vector vector, TscMode mode = TscMode::OneShot) -> void {
            auto lvt = read_reg(XApicRegType::LvtTimer);
            lvt &= 0xff;
            lvt |= static_cast<u32>(vector);
            lvt &= ~(BIT(16) | BIT(17));
            lvt |= static_cast<u32>(mode) << 16;
            write_reg(XApicRegType::LvtTimer, lvt);
        }

        FORCE_INLINE
        auto set_tsc_deadline(u64 deadline) -> void {
            MsrIo::write(MsrRegAddr::IA32TscDeadline, deadline);
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

        FORCE_INLINE
        auto mask(XApicRegType type) -> void {
            *get_reg_addr(type) |= kIntrMaskBit;
        }

        FORCE_INLINE
        auto unmask(XApicRegType type) -> void {
            *get_reg_addr(type) &= ~kIntrMaskBit;
        }

        FORCE_INLINE
        auto read_reg(XApicRegType type) -> u32 {
            return *get_reg_addr(type);
        }

        template <typename U32>
        FORCE_INLINE
        auto write_reg(XApicRegType type, U32 val) -> void {
            *get_reg_addr(type) = static_cast<u32>(val);
        }

        FORCE_INLINE
        auto get_reg_addr(XApicRegType type) const -> u32 volatile * {
            return (u32 volatile *)((u8 *)mmio_virt_ + u32(type));
        }

      private:
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

        CXX11_CONSTEXPR
        static auto const kIntrMaskBit = (u64(1) << 16);

        u32 *mmio_virt_;
        u8 id_;
        u8 logic_id_;
        u8 version_;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_APIC_XAPIC_HPP
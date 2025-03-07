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

#ifndef ARCH_X86_APIC_IOAPIC_HPP
#define ARCH_X86_APIC_IOAPIC_HPP 1

#include <arch/types.hpp>
#include <arch/interrupt.hpp>

namespace arch::x86 {
    enum class ApicIntrDelMode {
        // Unless you know what you're doing, you want FIXED.
        Fixed = 0,
        LowestPri = 1,
        Smi = 2,
        Nmi = 4,
        Init = 5,
        startup = 6,
        ExtInt = 7,
    };

    enum class ApicIntrDstMode {
        Physical = 0,
        Logical = 1,
    };

    struct IoApic {
        enum RegsAddr : u32 {
            Id,
            Version,
            RedTbl = 0x10,
        };

        IoApic(PhysAddr phys, VirtAddr addr)
            : mmio_virt_(reinterpret_cast<u32 *>(addr)),
              mmio_phys_(phys),
              id_((read(Id) >> 24) & 0xF) {
            auto const value = read(Version);
            version_ = value & 0xFF;
            max_local_irqnum_ = ((value >> 16) & 0xFF);
        }

        FORCE_INLINE
        auto read(RegsAddr addr) -> u32 {
            *get_reg_addr(IoRegSel) = addr;
            return *get_reg_addr(IoWindow);
        }

        FORCE_INLINE
        auto write(RegsAddr addr, u32 value) -> void {
            *get_reg_addr(IoRegSel) = addr;
            *get_reg_addr(IoWindow) = value;
        }

        FORCE_INLINE
        auto issue_eoi(u32 irqnum) -> Status {
            if (version_ < EOIREG_MIN_VERSION) {
                return Status::Unsupported;
            }

            *get_reg_addr(EoiReg) = irqnum;
            return Status::Ok;
        }

        FORCE_INLINE
        auto mask_all_irq() -> void {
            for (auto i = global_irqnum_base_; i <= max_local_irqnum_; ++i) {
                mask_irq(i);
            }
        }

        FORCE_INLINE
        auto mask_irq(u32 global_irqnum) -> void {
            u64 entry = read_redtbl(global_irqnum);
            entry |= REDTBL_ENTRY_MASKED;
            write_redtbl(global_irqnum, entry);
        }

        FORCE_INLINE
        auto unmask_irq(u32 global_irqnum) -> void {
            u64 entry = read_redtbl(global_irqnum);
            entry &= ~REDTBL_ENTRY_MASKED;
            write_redtbl(global_irqnum, entry);
        }

        FORCE_INLINE
        auto config_irq(u32 global_irqnum, IntrTriggerMode trimode, IntrPolarity polarity, ApicIntrDelMode delmode,
                        ApicIntrDstMode dstmode, u8 dst, u8 vector, bool mask = true) -> void {
            u64 entry = 0;
            entry |= vector;                    // Vector
            entry |= (u64(delmode) & 0xF) << 8; // Delivery Mode (Nmi, ...)
            entry |= u64(dstmode) << 11;        // Destination Mode (physical core or logical)
            entry |= u64(polarity) << 13;       // Pin Polarity (active low or high)
            entry |= u64(trimode) << 15;        // Trigger Mode (level or edge)
            entry |= u64(mask) << 16;           // Mask (Enabled or disabled)
            entry |= u64(dst) << 56;            // Destination (Which processor)

            write_redtbl(global_irqnum, entry);
        }

        FORCE_INLINE
        auto id() -> u32 {
            return id_;
        }

        FORCE_INLINE
        auto version() -> u32 {
            return version_;
        }

      private:
        enum CtrlRegType {
            IoRegSel = 0,
            IoWindow = 0x10,
            EoiReg = 0x40,
        };

        FORCE_INLINE
        auto get_reg_addr(CtrlRegType type) -> u32 volatile * {
            return reinterpret_cast<u32 volatile *>(reinterpret_cast<u8 *>(mmio_virt_) + type);
        }

        FORCE_INLINE
        auto read_redtbl(u32 global_irqnum) -> u64 {
            DEBUG_ASSERT(global_irqnum > global_irqnum_base_);
            u32 const local_irqnum = global_irqnum - global_irqnum_base_;
            DEBUG_ASSERT(local_irqnum <= max_local_irqnum_);

            RegsAddr const redtbl_low = RegsAddr(RedTbl + (global_irqnum << 1));
            RegsAddr const redtbl_high = RegsAddr(redtbl_low + 1);
            u64 value = read(redtbl_low);
            value |= u64(read(redtbl_low)) << 32;
            return value;
        }

        FORCE_INLINE
        auto write_redtbl(u32 global_irqnum, u64 value) -> void {
            DEBUG_ASSERT(global_irqnum > global_irqnum_base_);
            u32 const local_irqnum = global_irqnum - global_irqnum_base_;
            DEBUG_ASSERT(local_irqnum <= max_local_irqnum_);

            RegsAddr const redtbl_low = RegsAddr(RedTbl + (global_irqnum << 1));
            RegsAddr const redtbl_high = RegsAddr(redtbl_low + 1);
            write(redtbl_low, value & 0xFFFFFFFF);
            write(redtbl_high, (value >> 32) & 0xFFFFFFFF);
        }

        CXX11_CONSTEXPR
        static auto const EOIREG_MIN_VERSION = 0x20;

        CXX11_CONSTEXPR
        static auto const REDTBL_ENTRY_MASKED = (u64(1) << 16);

        /// Holds the base address of the registers in virtual memory. This
        /// address is `non-cacheable` (see paging).
        ai_virt u32 *mmio_virt_;
        PhysAddr mmio_phys_;
        u8 id_;
        u8 version_;
        u8 max_local_irqnum_;

        /// The first IRQ which this IOAPIC handles. This is only found in the
        /// IOAPIC entry of the ACPI 2.0 MADT. It isn't found in the IOAPIC
        /// registers.
        u32 global_irqnum_base_;
    };

} // namespace arch::x86

#endif // #ifndef ARCH_X86_APIC_IOAPIC_HPP
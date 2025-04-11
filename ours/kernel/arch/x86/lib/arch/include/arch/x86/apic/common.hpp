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
#ifndef ARCH_X86_APIC_COMMON_HPP
#define ARCH_X86_APIC_COMMON_HPP 1

#include <arch/types.hpp>

namespace arch {
    enum class ApicInterruptPolarity {
        ActiveLow,
        ActiveHigh,
    };

    enum class ApicDeliveryMode {
        /// The standard mode for delivering interrupts. The interrupt is delivered 
        /// directly to the target CPU as specified in the APIC configuration.
        /// Unless you have a specific reason to use another mode, this is the default choice.
        Fixed = 0,

        /// The interrupt is delivered to the CPU with the lowest priority among 
        /// the available target CPUs. Useful for load balancing in multiprocessor systems.
        LowestPri = 1,

        /// System Management Interrupt (SMI). Forces the target CPU into System 
        /// Management Mode (SMM), where it runs firmware-level code (e.g., BIOS routines).
        /// This is typically used for power management or hardware maintenance tasks.
        Smi = 2,

        /// Non-Maskable Interrupt (NMI). This interrupt cannot be blocked by the CPU 
        /// and is used for critical events like hardware failures or watchdog mechanisms.
        Nmi = 4,

        /// INIT signal. Places the target CPU in an initialization state but does not 
        /// execute any code. Used during multiprocessor startup sequences.
        Init = 5,

        /// Startup Inter-Processor Interrupt (SIPI). Used to start a CPU by making it 
        /// jump to a specific memory address. Typically follows an INIT signal 
        /// in multiprocessor boot sequences.
        Startup = 6,

        /// External Interrupt (ExtInt). Used to forward interrupts from legacy 
        /// 8259 Programmable Interrupt Controllers (PIC) to the CPU via the APIC.
        /// Ensures backward compatibility with older systems that do not fully use APIC.
        ExtInt = 7,
    };

    enum class ApicDeliveryStatus {
        Idle,
        Pending,
    };

    enum class ApicDestinationMode {
        Physical = 0,
        Logical = 1,
    };

    enum class ApicTriggerMode {
        Edge = 0,
        Level = 1,
    };

    enum class ApicIpiTarget {
        TheOne,
        Self,
        AllButSelf,
        All,
    };

    enum class ApicIpiLevel {
        Deassert,
        Assert,
    };

    struct IpiRequest {
        typedef IpiRequest Self;

        IpiRequest() = default;

        // clang-format off
        IpiRequest(u32 vector, u32 apic_id,
                   ApicIpiTarget target = ApicIpiTarget::TheOne,
                   ApicTriggerMode trimode = ApicTriggerMode::Edge,
                   ApicDeliveryMode delmode = ApicDeliveryMode::Nmi, 
                   ApicDestinationMode dstmode = ApicDestinationMode::Physical, 
                   ApicDeliveryStatus delstatus = ApicDeliveryStatus::Pending,
                   ApicIpiLevel level = ApicIpiLevel::Assert)
            : wrapper_(0) {
            set_vector(vector).set_delivery_mode(delmode);
        }
        // clang-format on 

        FORCE_INLINE
        auto set_vector(u32 vector) -> Self & {
            wrapper_ |= vector;
            return *this;
        }

        FORCE_INLINE
        auto set_delivery_mode(ApicDeliveryMode mode) -> Self &{
            wrapper_ |= u64(mode) << 8;
            return *this;
        }

        FORCE_INLINE
        auto set_dest_mode(ApicDestinationMode mode) -> Self & {
            wrapper_ |= u64(mode) << 11;
            return *this;
        }

        FORCE_INLINE
        auto set_delivery_status(ApicDeliveryStatus status) -> Self & {
            wrapper_ |= u64(status) << 12;
            return *this;
        }

        FORCE_INLINE
        auto set_level(ApicIpiLevel level) -> Self & {
            wrapper_ |= u64(level) << 14;
            return *this;
        }

        FORCE_INLINE
        auto set_trigger_mode(ApicTriggerMode mode) -> Self & {
            wrapper_ |= u64(mode) << 15;
            return *this;
        }

        FORCE_INLINE
        auto set_target(ApicIpiTarget target) -> Self & {
            wrapper_ |= u64(target) << 18;
            return *this;
        }

        FORCE_INLINE
        auto to_icr0() const -> u32 {
            return wrapper_;
        }

        FORCE_INLINE
        auto to_icr1() const -> u32 {
            return (wrapper_ >> 32);
        }

        u64 wrapper_;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_APIC_COMMON_HPP
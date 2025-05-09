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
#ifndef OURS_ARCH_APIC_HPP
#define OURS_ARCH_APIC_HPP 1

#include <ours/init.hpp>
#include <ours/irq/irq_chip.hpp>

#include <arch/x86/interrupt.hpp>
#include <ustl/views/span.hpp>
#include <arch/x86/apic/ioapic.hpp>

namespace ours {
    using arch::ApicDeliveryMode;
    using arch::ApicDestinationMode;
    using arch::ApicTriggerMode;

    INIT_CODE
    auto init_io_apic(ustl::views::Span<arch::IoApic> const &ioapics,
                      ustl::views::Span<arch::IoApicIsaOverride> const &overrides) -> void;

    /// Called once after VMM initialized.
    INIT_CODE
    auto init_local_apic() -> void;

    INIT_CODE
    auto init_local_apic_percpu() -> void;

    INIT_CODE
    auto init_apic_deadline_tsc() -> void;

    auto apic_timer_set_oneshot(u32 n, u8 divisor, bool mask) -> Status;

    auto apic_timer_set_tsc_deadline(Ticks deadline) -> void;

    auto apic_timer_current_count() -> Ticks;

    auto apic_send_ipi(CpuNum target, arch::IrqVec vector, ApicDeliveryMode mode) -> void;

    auto apic_send_ipi_mask(CpuMask targets, arch::IrqVec vector, ApicDeliveryMode mode) -> void;

    auto apic_broadcast_ipi(arch::IrqVec vector, ApicDeliveryMode mode) -> void;

    auto apic_mask_irq(HIrqNum global_irq) -> void;

    auto apic_unmask_irq(HIrqNum global_irq) -> void;

    auto apic_configure_irq(HIrqNum global_irq, arch::ApicTriggerMode,
                            arch::ApicInterruptPolarity, arch::ApicDeliveryMode,
                            bool mask, arch::ApicDestinationMode, u8 dst, arch::IrqVec vector) -> void;

    auto apic_configure_isa_irq(u8 isa_irq, arch::ApicDeliveryMode del_mode, bool mask,
                                arch::ApicDestinationMode dst_mode, u8 dst, arch::IrqVec vector) -> void;

    auto cpunum_to_apicid(CpuNum cpunum) -> u32;
    auto current_apic_id() -> u32;

    extern irq::IrqChip *g_ioapic_chip;

} // namespace ours

#endif // #ifndef OURS_ARCH_APIC_HPP
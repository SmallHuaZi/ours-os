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

#include <arch/types.hpp>

namespace arch::x86 {
    /// The Intel Software Developer's Manual, however states that,
    /// once you have disabled the local APIC through IA32_APIC_BASE
    /// you can't enable it anymore until a complete reset. The I/O APIC
    /// can also be configured to run in legacy mode so that it emulates
    /// an 8259 device.
    struct XApic {

      private:
        ai_virt u32 *mmio_virt_;
    };

} // namespace arch::x86

#endif // #ifndef ARCH_X86_APIC_XAPIC_HPP
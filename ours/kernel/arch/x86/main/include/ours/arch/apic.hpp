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

#include <ustl/views/span.hpp>
#include <arch/x86/apic/ioapic.hpp>

namespace ours {
    INIT_CODE
    auto init_io_apic(ustl::views::Span<arch::IoApic> &ioapics,
                      ustl::views::Span<arch::IoApicIsaOverride> &overrides) -> void;

    INIT_CODE
    auto init_local_apic() -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_APIC_HPP
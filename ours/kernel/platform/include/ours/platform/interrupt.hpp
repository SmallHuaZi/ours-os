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
#ifndef OURS_PLATFORM_INTERRUPT_HPP
#define OURS_PLATFORM_INTERRUPT_HPP 1

#include <ours/types.hpp>

#include <arch/x86/interrupt.hpp>

namespace ours {
    auto platform_handle_irq(HIrqNum irqnum, arch::IrqFrame *) -> void;

} // namespace ours

#endif // #ifndef OURS_PLATFORM_INTERRUPT_HPP
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
#ifndef OURS_ARCH_X86_IDT_HPP
#define OURS_ARCH_X86_IDT_HPP 1

#include <arch/x86/interrupt.hpp>

namespace ours {
    auto x86_init_idt_early() -> void;

    auto x86_setup_idt() -> void;

    auto x86_load_idt() -> void;

    auto x86_dump_idt() -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_X86_IDT_HPP
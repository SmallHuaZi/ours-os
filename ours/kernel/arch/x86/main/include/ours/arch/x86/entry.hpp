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
#ifndef OURS_ARCH_X86_ENTRY_HPP
#define OURS_ARCH_X86_ENTRY_HPP 1

#include <ours/config.hpp>

#include <arch/x86/interrupt.hpp>

namespace ours {
    NO_MANGLE 
    auto x86_handle_syscall(arch::IrqFrame *) -> void;

    NO_MANGLE 
    auto x86_enter_uspace(arch::IrqFrame *) -> void;

    NO_MANGLE 
    auto x86_switch_context(VirtAddr *oldsp, VirtAddr newsp) -> void;

} // namespace ours

#endif // #ifndef OURS_ARCH_X86_ENTRY_HPP
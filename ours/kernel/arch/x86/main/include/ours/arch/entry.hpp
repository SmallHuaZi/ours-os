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
#ifndef OURS_ARCH_ENTRY_HPP
#define OURS_ARCH_ENTRY_HPP 1

#include <ours/arch/x86/entry.hpp>
#include <arch/macro/system.hpp>

namespace ours {
    NO_RETURN FORCE_INLINE 
    static auto arch_enter_uspace(arch::IrqFrame *frame) -> void {
        x86_enter_uspace(frame);
    }

    FORCE_INLINE
    static auto arch_build_uspace_irqframe(arch::IrqFrame *frame, usize ip, usize sp, usize arg1, usize arg2) -> void {
        using arch::ArchCpuState;

        frame->ip = ip;
        frame->usp = sp;

        frame->cs = X86_GDT_USER_CODE64;
        frame->uss = X86_GDT_USER_DATA;
        frame->flags.set<ArchCpuState::IOPL>(1) // Enable pesmission level
                    .set<ArchCpuState::IF>(1);  // Enable interrupt

        // Don't modify, it is required.
        frame->di = arg1;
        frame->si = arg2;
    }

} // namespace ours

#endif // #ifndef OURS_ARCH_ENTRY_HPP
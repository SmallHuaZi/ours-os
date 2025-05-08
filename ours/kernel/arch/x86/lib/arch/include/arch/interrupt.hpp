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
#ifndef ARCH_INTERRUPT_HPP
#define ARCH_INTERRUPT_HPP 1

#include <arch/types.hpp>
#include <arch/processor/cpu-states.hpp>
#include <arch/x86/interrupt.hpp>

#include <ustl/sync/atomic.hpp>

namespace arch {
    typedef ArchCpuState   IntrSavedState;

    FORCE_INLINE
    static auto disable_interrupt() -> void {
        asm ("cli;" ::: "memory");
    }

    FORCE_INLINE
    static auto enable_interrupt() -> void {
        asm ("sti;" ::: "memory");
    }

    CXX11_CONSTEXPR
    static auto const kNullIntrSavedState = IntrSavedState();

    FORCE_INLINE
    static auto save_interrupt_state() -> IntrSavedState {
        auto state = ArchCpuState::read();
        if (state.get<state.IF>()) {
            // If the interrupt enable bit was set, clear it and write back.
            state.set<state.IF>(0)
                 .write()
                 .set<state.IF>(1);
        }

        // Prevent CPU's disorder performing behaviour
        atomic_signal_fence(ustl::sync::MemoryOrder::SeqCst);
        return state;
    }

    FORCE_INLINE
    static auto restore_interrupt_state(IntrSavedState state) -> void {
        // Prevent CPU's disorder performing behaviour
        atomic_signal_fence(ustl::sync::MemoryOrder::SeqCst);

        state.write();
    }

} // namespace arch

#endif // #ifndef ARCH_INTERRUPT_HPP
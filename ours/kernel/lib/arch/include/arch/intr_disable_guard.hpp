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

#ifndef ARCH_INTR_DISABLE_GUARD_HPP
#define ARCH_INTR_DISABLE_GUARD_HPP 1

#include <arch/interrupt.hpp>

namespace arch {
    /// RAII Object to enable/disable interrupt handler.
    struct IntrDisableGuard {
        IntrDisableGuard()
            : state_(save_interrupt_state()) {}

        ~IntrDisableGuard() {
            reenable();
        }

        auto reenable() -> void {
            restore_interrupt_state(state_);
            state_ = kNullIntrSavedState;
        }

        IntrSavedState state_;
    };

} // namespace arch

#endif // #ifndef ARCH_INTR_DISABLE_GUARD_HPP
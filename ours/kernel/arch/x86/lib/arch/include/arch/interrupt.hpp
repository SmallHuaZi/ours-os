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

namespace arch {
    typedef usize   IntrSavedState;

    CXX11_CONSTEXPR
    static auto const NULL_INTR_SAVED_STATE = IntrSavedState();

    FORCE_INLINE
    auto save_interrupt_state() -> IntrSavedState {
    }

    FORCE_INLINE
    auto restore_interrupt_state(IntrSavedState) -> void {
    }

} // namespace arch

#endif // #ifndef ARCH_INTERRUPT_HPP
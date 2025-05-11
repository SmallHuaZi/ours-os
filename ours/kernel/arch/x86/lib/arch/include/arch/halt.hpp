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
#ifndef ARCH_HALT_HPP
#define ARCH_HALT_HPP 1

#include <arch/types.hpp>

namespace arch {
    FORCE_INLINE
    static auto halt() -> void {
        asm volatile("hlt");
    }

    FORCE_INLINE
    static auto pause() -> void {
        asm volatile("pause");
    }

    FORCE_INLINE
    static auto suspend() -> void {
        asm volatile("hlt; pause");
    }

} // namespace arch

#endif // #ifndef ARCH_HALT_HPP
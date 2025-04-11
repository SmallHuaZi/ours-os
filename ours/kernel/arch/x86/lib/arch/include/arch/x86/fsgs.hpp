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
#ifndef ARCH_X86_FSGS_HPP
#define ARCH_X86_FSGS_HPP 1

#include <arch/types.hpp>

namespace arch {
    template <typename Integer>
    FORCE_INLINE
    static auto read_gs_offset(usize offset) -> Integer {
        Integer ret;
        asm volatile("mov %%gs:%1, %0" : "=r"(ret) : "m"(*(Integer *)offset) : "memory");
        return ret;
    }

    template <typename Integer>
    FORCE_INLINE
    static auto write_gs_offset(usize offset, Integer val) -> void {
        asm volatile("mov %0, %%gs:%1" :: "ir"(val), "m"(*(Integer *)offset) : "memory");
    }

} // namespace arch

#endif // #ifndef ARCH_X86_FSGS_HPP
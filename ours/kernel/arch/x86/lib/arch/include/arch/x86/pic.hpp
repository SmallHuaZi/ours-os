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

#ifndef ARCH_X86_PIC_HPP
#define ARCH_X86_PIC_HPP 1

#include <arch/types.hpp>

namespace arch::x86 {
    struct Pic {
        static auto send_eoi(u8 irq) -> void;
        static auto remap(u8 pic1, u8 pic2) -> void;
        static auto wait_io() -> void;
        static auto mask(u8 vector) -> void;
        static auto mask_all() -> void;
        static auto unmask(u8 vector) -> void;
        static auto unmask_all() -> void;
    };

} // namespace arch::x86

#endif // #ifndef ARCH_X86_PIC_HPP
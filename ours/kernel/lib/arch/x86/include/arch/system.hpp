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

#ifndef ARCH_X86_SYSTEM_HPP
#define ARCH_X86_SYSTEM_HPP 1

#include <ours/types.hpp>

namespace arch {
    using ours::usize;

    template <typename Derived>
    struct ControlRegister 
    {
        typedef ControlRegister     Self;

        static auto read() -> usize
        {
            usize out;
            asm volatile("mov %%cr3, %0" :"=m"(out) :: "memory");
            return out;
        }

        static auto write(usize in) -> void 
        {
            asm volatile("mov %%cr3, %0" :: "m"(in): "memory");
        }
    };

    struct Cr0
        : public ControlRegister<Cr0>
    {
        enum BitFileds: usize {
            X87,
            Sse,
            Avx,
            BndReg,
            BndCsr,
            OpMask,
            ZmmHi256,
            Hi16Zmm,
            __P0,
            Pkru
        };
    };

    struct Cr1
    {};

    struct Cr2
    {};

    struct Cr3
    {};

} // namespace arch::x86

#endif // #ifndef ARCH_X86_SYSTEM_HPP
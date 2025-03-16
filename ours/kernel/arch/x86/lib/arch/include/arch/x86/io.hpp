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

#ifndef ARCH_X86_IO_HPP
#define ARCH_X86_IO_HPP 1

#include <arch/types.hpp>

namespace arch::x86 {
    FORCE_INLINE 
    auto outb(u16 port, u8 val) -> void 
    { asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }

    FORCE_INLINE 
    auto inb(u16 port) -> u8 
    {
        u8 ret;
        asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    FORCE_INLINE 
    auto outw(u16 port, u16 val) -> void 
    {  asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));  }

    FORCE_INLINE 
    auto inw(u16 port) -> u16 
    {
        u16 ret;
        asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    FORCE_INLINE 
    auto outl(u16 port, u32 val) -> void 
    { asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port)); }

    FORCE_INLINE 
    auto inl(u16 port) -> u32
    {
        u32 ret;
        asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

} // namespace arch::x86

#endif // #ifndef ARCH_X86_IO_HPP
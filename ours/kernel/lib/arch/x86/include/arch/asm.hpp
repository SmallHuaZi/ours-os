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

#ifndef ARCH_ASM_HPP
#define ARCH_ASM_HPP 1

.macro reg_map reg
    .if __x86_64__
        %r\reg
    .else
        %e\reg
    .endif
.endm

.macro push_reg reg
    push reg_map reg 
    .cfi_adjust_cfa_offset 8
    .cfi_rel_offset reg_map reg, 0
.endm

.macro pop_reg reg
    pop reg_map reg
    .cfi_adjust_cfa_offset -8
    .cfi_same_value reg_map reg
.endm

.macro save_general_registers
    push_reg 15
    push_reg 14
    push_reg 13
    push_reg 12
    push_reg 11
    push_reg 10
    push_reg 9
    push_reg 8
    push_reg ax
    push_reg cx
    push_reg dx
    push_reg bx
    push_reg bp
    push_reg si
    push_reg di
.endm

.macro restore_general_registers
    pop_reg di
    pop_reg si
    pop_reg bp
    pop_reg bx
    pop_reg dx
    pop_reg cx
    pop_reg ax
    pop_reg 8
    pop_reg 9
    pop_reg 10
    pop_reg 11
    pop_reg 12
    pop_reg 13
    pop_reg 14
    pop_reg 15
.endm

.macro zero_general_registers
    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %ecx, %ecx
    xorl %edx, %edx
    xorl %esi, %esi
    xorl %edi, %edi
    xorq %r8, %r8
    xorq %r9, %r9
    xorq %r10, %r10
    xorq %r11, %r11
    xorq %r12, %r12
    xorq %r13, %r13
    xorq %r14, %r14
    xorq %r15, %r15
.endm

#endif // #ifndef ARCH_ASM_HPP
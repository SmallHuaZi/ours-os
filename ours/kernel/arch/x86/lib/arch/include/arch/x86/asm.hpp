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
#ifndef ARCH_X86_ASM_HPP
#define ARCH_X86_ASM_HPP 1

.macro push_reg reg
    push \reg 
    .cfi_adjust_cfa_offset 8
    .cfi_rel_offset \reg, 0
.endm

.macro push_val val
    push \val
    .cfi_adjust_cfa_offset 8
.endm

.macro pop_reg reg
    pop \reg
    .cfi_adjust_cfa_offset -8
    .cfi_same_value \reg
.endm

.macro save_general_registers
    push_reg %r15
    push_reg %r14
    push_reg %r13
    push_reg %r12
    push_reg %r11
    push_reg %r10
    push_reg %r9
    push_reg %r8
    push_reg %rax
    push_reg %rcx
    push_reg %rdx
    push_reg %rbx
    push_reg %rbp
    push_reg %rsi
    push_reg %rdi
.endm

.macro restore_general_registers
    pop_reg %rdi
    pop_reg %rsi
    pop_reg %rbp
    pop_reg %rbx
    pop_reg %rdx
    pop_reg %rcx
    pop_reg %rax
    pop_reg %r8
    pop_reg %r9
    pop_reg %r10
    pop_reg %r11
    pop_reg %r12
    pop_reg %r13
    pop_reg %r14
    pop_reg %r15
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

.macro all_cfi_same_value 
    .cfi_same_value %rax 
    .cfi_same_value %rbx 
    .cfi_same_value %rcx 
    .cfi_same_value %rdx 
    .cfi_same_value %rsi 
    .cfi_same_value %rdi 
    .cfi_same_value %rbp 
    .cfi_same_value %r8
    .cfi_same_value %r9 
    .cfi_same_value %r10 
    .cfi_same_value %r11 
    .cfi_same_value %r12 
    .cfi_same_value %r13  
    .cfi_same_value %r14 
    .cfi_same_value %r15 
.endm // all_cfi_same_value

.macro add_to_sp value
    add $\value, %rsp
    .cfi_adjust_cfa_offset -\value
.endm // add_to_sp  

.macro sub_from_sp value
    sub $\value, %rsp
    .cfi_adjust_cfa_offset \value
.endm // sub_from_sp

#endif // #ifndef ARCH_X86_ASM_HPP
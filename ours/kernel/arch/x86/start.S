#include <ours/arch/x86/constant.hpp>

    .extern x86_main
    .extern setup_gdt
    .extern G_GDT

    .section .early.text

    .globl x86_start
x86_start:
    cli
    mov $KERNEL_STACK, %esp

    // Buffer the pointer to the multiboot_info and the bootloader-magic.
    push %eax   // Magic
    push %ebx   // Multiboot2 information structure

    call init_static_objects
    call setup_gdt

    // Reset %DS
    movl $OURS_GDT_KERNEL_DATA, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs

    // Reset %CS
    // pushfl
    // orl $0x200, (%esp)
    // pushl $OURS_GDT_KERNEL_CODE32 
    // pushl $1f
    // iret

    ljmpl $OURS_GDT_KERNEL_CODE32, $1f 
1:
    call x86_main 

idle:
    cli
    hlt
    jmp idle

.section .bss
.space 2 * 1024 * 1024; # 2 MiB
KERNEL_STACK:
#include <ours/arch/asm.hpp>
#include <ours/macro_abi.hpp>

// [[no_return]] auto x86_start(LegacyBootParam %esi) -> void
.code32
FUNCTION_NOCFI(_start32)
    // Disable interruptions.
    cli

    // Clear .bss section whose alignment was assumeed to 4.
    cld
    xor %eax, %eax
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    sub %edi, %ecx
    shr $2, %ecx
    rep stosl

    // Load ealry stack
    mov $g_early_stack_end, %esp

    // Clear the IDT to zero address and zero limit, so any trap is sure to
    // get a triple-fault.  We've just cleared the .bss containing the stack,
    // so these bytes are known to be zero.
    lidt -6(%esp)
    
    // Clear EFLAGS
    push %eax
    popf

    // Load null ldt.
    lldt %ax

    // Build a temporary descriptor pointing at the GDT to load it.
    lgdt g_temp_gdtr

    // Reload CS 
    ljmp $X86_GDT_KERNEL_CODE32, $0f

0:
    mov $X86_GDT_KERNEL_DATA, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    xor %eax, %eax
    mov %ax, %fs

    xor %ebp, %ebp

    push %esi
    call phys_main

idle:
    cli
    hlt
    jmp idle 

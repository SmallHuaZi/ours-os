#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/aspace_layout.hpp>
#include <ours/arch/asm.hpp>

#include <ours/macro_abi.hpp>
#include <ours/mem/cfg.hpp>

#include <arch/x86/registers.hpp>

// For load GDTR
.extern PHYS_PGD 
.extern TEMP_GDTR
.extern KERNEL_VIRT_BASE
.extern KERNEL_PHYS_BASE

// Kernel entry point
.extern start_kernel

/// [[NO_RETURN]] 
/// auto _start(VirtAddr relocated_base, PhysAddr handoff) -> void
/// %rdi: relocated_base
/// %rsi: handoff
.align 8
.function _start, global
    // Disable interrupts
    cli

    // Save handoff
    mov %rsi, %r15

    // Page table has been changed, so we must reload the data before
    // to rectify their address.
    lea KERNEL_STACK_END(%rip), %rsp

    lgdt TEMP_GDTR(%rip)

    // Save the relocated_base of our main module 
    mov %rdi, KERNEL_VIRT_BASE(%rip)

    pushq $X86_GDT_KERNEL_CODE64
    lea .Lreload_cs(%rip), %rax
    pushq %rax
    lretq

// Now we is on the high address. For ASLR, the code bottom is required being PIC.
.Lreload_cs:
    // Zero our kernel segment data registers
    xor %eax, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    // Call our main module
    mov %r15, %rdi  // Pass handoff
    call start_kernel

idle:
    hlt
    pause
    jmp idle
.end_function

.object KERNEL_STACK, bss, global, align=16
    .skip KB(2)
KERNEL_STACK_END:
.end_object

.label MAIN_ENTRY, global, value=_start
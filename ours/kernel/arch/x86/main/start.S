#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/aspace_layout.hpp>
#include <ours/arch/asm.hpp>

#include <ours/macro_abi.hpp>
#include <ours/mem/cfg.hpp>

/// The symbols below should be defined in *.cpp files. It is because of
/// controlling the size of them for difference in 32-bits and 64-bits platform.
.extern g_kernel_virt_base
.extern g_kernel_phys_base
.extern g_kernel_stack_bottom

// Kernel entry point
.extern start_kernel

/// [[NO_RETURN]] 
/// auto _start(PhysAddr handoff) -> void
/// %rdi: relocated_base
.align 8
.function _start, global
    // Disable interrupts
    cli

    // Save handoff
    mov %rdi, %r15

    mov %rsp, g_kernel_stack_bottom(%rip)
    mov %cr3, %rax
    mov %rax, g_pgd(%rip)

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

/// Export for main.ld
.label MAIN_ENTRY, global, value=_start
#include <ours/arch/asm.hpp>
#include <ours/arch/x86/bootstrap.hpp>
#include <ours/arch/x86/descriptor.hpp>

#include <arch/macro/system.hpp>
#include <arch/macro/msr.hpp>

/// Why do we need to use this?
///
/// Because under real mode, we are limited to just access the memory lower than
/// 1 MB, but our main module which include the following code was loaded at address 
/// which is over N GB. So that The following code must be copied to low address,
/// usually first k pages, therefore there is a new problem related with relocation. 
/// To bypass it and avoid so many relocating work, we can use this tricky from Linux 
/// to have a relative addressing way.
///
#define rva(symbol) (symbol - x86_start_bootstrap16)

.label __x86_bootstrap16_start, global

/// As <ours/arch/x86/bootstrap.hpp> said, the BootstrapData 
/// was put at address 0.
.code16
.label x86_start_bootstrap16, global
    // Store the real address of `x86_start_bootstrap16`
    mov X86_BOOTSTRAP_OFFSET_BOOTSTRAP_ENTRY, %ebx

    mov %cr0, %eax
    or $X86_CR0_CD, %eax
    and $~X86_CR0_NW, %eax
    mov %eax, %cr0

    lgdt X86_BOOTSTRAP_OFFSET_GDTR

    // Enter protected mode without paging
    mov %cr0, %eax
    or $X86_CR0_PE, %eax
    mov %eax, %cr0

    // Enable Paging
    mov %cr4, %eax
    or $(X86_CR4_PAE | X86_CR4_PGE), %eax
    mov %eax, %cr4

    // Load CR3 with the PGD (It's address is in 32bits)
    mov X86_BOOTSTRAP_OFFSET_PGD, %eax
    mov %eax, %cr3

    // Enable long mode
    mov $X86_MSR_IA32_EFER, %ecx
    rdmsr
    or $X86_EFER_LME, %eax
    or $X86_EFER_NXE, %eax
    wrmsr

    // Enable Paging
    mov %cr4, %eax
    or $(X86_CR4_PAE | X86_CR4_PGE), %eax
    mov %eax, %cr4

    // Reload CS and jump to 64-bit code.
    mov X86_GDT_KERNEL_CODE64, %eax
    mov %eax, %cs
    lea rva(x86_secondary_cpu_long_mode_entry)(%ebx), %esp
    lret

.code64
.function x86_secondary_cpu_long_mode_entry, local 

    // Zero our data segments
    xor %eax, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    // FIXME(SmallHuaZi): This should be as an argument but a real symbol reference.
    call x86_start_nonboot_cpu
0:
    hlt
    jmp 0b
.end_function

.label __x86_bootstrap16_end, global 
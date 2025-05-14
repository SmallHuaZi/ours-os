#include <ours/arch/asm.hpp>
#include <ours/arch/x86/bootstrap.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/mem/cfg.hpp>

#include <arch/macro/system.hpp>
#include <arch/macro/msr.hpp>

/// Why do we need to use this?
///
/// Because under real mode, we are limited to just access the memory lower than
/// 1 MB, but our main module which include the following trampoline code was loaded 
/// at address which is over N GB. So that The following code must be copied to low 
/// address, usually first k pages, therefore there is a new problem related with relocation. 
/// To bypass it and avoid so many relocating work, we can use this tricky from Linux 
/// to have a relative addressing way.
///
#define rva(symbol) (symbol - __rva_start)

.section ".init.boot.head", "ax", %progbits
.balign PAGE_SIZE
.label __x86_bootstrap16_start, global

.code16
.label x86_start_bootstrap16, global
    cli
    cld

    // Non-cache mode.
    mov %cr0, %eax
    or $X86_CR0_CD, %eax
    and $~X86_CR0_NW, %eax
    mov %eax, %cr0

    mov %cs, %si
    // Let ds and ss use the 2nd page.
    add $0x100, %si
    mov %si, %ds
    mov %si, %ss

    lgdtl X86_BOOTSTRAP_OFFSET_GDTR

    // Enter protected mode without paging
    mov %cr0, %eax
    or $X86_CR0_PE, %eax
    mov %eax, %cr0

    // Enable PAE and PGE 
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

    // enable paging
    mov %cr0, %eax
    or $X86_CR0_PG, %eax
    mov %eax, %cr0

//     // Translate data page segment into full address
    mov %ds, %ebx
    shl $4, %ebx

    // Reload CS and jump to 64-bit code.
    mov $X86_BOOTSTRAP_OFFSET_LONGMODE_PHYS_ENTRY, %esp // ss:X86_BOOTSTRAP_OFFSET_LONGMODE_PHYS_ENTRY
    lretl

.code64
.label x86_nonboot_cpu_enter_long_mode, global 
    // Zero our data segments
    xor %eax, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    // Let %rsi=cpunum
    xor %rsi, %rsi
    movl X86_BOOTSTRAP_OFFSET_CPUNUM(%rbx), %esi

    // Stack top: %rsi + X86_BOOTSTRAP_OFFSET_PERCPU + 8*2n;
    // Thread pointer: %rsi + X86_BOOTSTRAP_OFFSET_PERCPU + 8*(2n + 1);

    // Rectifies the index.
    mov %rsi, %rax
    dec %rax
    shl $1, %rax
.label __debug_symbol0, global 
    // Let %rsp point to kernel stack of current thread.
    mov X86_BOOTSTRAP_OFFSET_PERCPU(%rbx, %rax, 8), %rsp

.label __debug_symbol1, global 
    add $1, %rax
.label __debug_symbol2, global
    // Let %rdi point to current thread
    mov X86_BOOTSTRAP_OFFSET_PERCPU(%rbx, %rax, 8), %rdi

.label __debug_symbol3, global
.label __debug_symbol4, global
    //
    jmp *X86_BOOTSTRAP_OFFSET_LONGMODE_VIRT_ENTRY(%rbx)
0:
    hlt
    jmp 0b

.label __x86_bootstrap16_end, global 
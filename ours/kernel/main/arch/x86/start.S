#include <ours/arch/x86/mmu.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <ours/asm.hpp>
#include <ours/marco_abi.hpp>
#include <ours/mem/cfg.hpp>

#include <arch/x86/registers.hpp>

#define NR_PHYSMAP_PTES   (ARCH_PHYSMAP_SIZE / MAX_PAGE_SIZE)
#define NR_PHYSMAP_PDS    (NR_PHYSMAP_PTES / 512)
#define PDP_HIGH_OFFSET (512 - KERNEL_ASPACE_SIZE / GB(1))

// For load GDTR
.extern TEMP_GDTR 

// Pre-allocated page table entries which defined at "arch/x86/vm_aspace.cpp",
// avabilable after the BSS was zeroed
.extern KERNEL_PGD
.extern KERNEL_PDP
.extern KERNEL_PD
.extern KERNEL_PDP_HIGH
.extern KERNEL_PHYSMAP_PD

.extern KERNEL_VIRT_BASE
.extern KERNEL_PHYS_BASE

// Kernel entry point
.extern start_kernel

// The default layout:
// |   63-48   | 47-39 (PML4) | 38-30 (PDP) | 29-21 (PD) | 20-12 (PT) | 11-0 (Offset) |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   9 bits   |    12 bits    |
//
// For PT: KERNEL_VIRT_BASE[(12 / 8) = 1]   | 63...8 |
// |   63-40   | 39-31 (PML4) | 30-22 (PDP) | 21-13 (PD) | 12-4 (PT) | 3-0 (Offset) |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   9 bits  |    4 bits    |
//
// For PD: KERNEL_VIRT_BASE[(21 / 8) = 2]   | 63...16 |
// |   63-32   | 31-23 (PML4) | 22-14 (PDP) |  13-5 (PD) | 4-0 (PT)  |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   5 bits  |
//
// For PDP: KERNEL_VIRT_BASE[(30 / 8) = 3]  | 63...24 |
// |   63-24   | 23-15 (PML4) | 14-6 (PDP)  |  5-0 (PD)  |
// |    ..     |    9 bits    |    9 bits   |   6 bits   |
//
// For PML4: KERNEL_VIRT_BASE[(39 / 8) = 4] | 63...32 |
// |   63-16   | 15-7 (PML4)  |  6-0 (PDP)  |
// |    ..     |    9 bits    |    7 bits   |
//
#define ADDR_OFFSET 9
#define ADDR_OFFSET_MASK ((1 << ADDR_OFFSET) - 1)
#define SHIFT_OFFSET(shift) (shift >> 3)
#define SHIFT_REMAIN(shift) ((shift) - (SHIFT_OFFSET(shift) << 3))

// table: Virtual address
// shift: 
// pte: Register which store the PTE value
.macro install_relocated_pte table, shift, pte
    mov PHYS(KERNEL_VIRT_BASE + SHIFT_OFFSET(\shift)), %ecx
    shrl $SHIFT_REMAIN(\shift), %ecx
    andl $ADDR_OFFSET_MASK, %ecx

    // Get the real entry
    shl $3, %ecx
    add $PHYS(\table), %ecx
    mov \pte, (%ecx) 
.endm

// [[NO_RETURN]] auto _start(VirtAddr relocated_base, PhysAddr handoff) -> void
// %rdi: relocated_base
// %rsi: handoff

.section .init.boot, "ax", @progbits
#ifdef CHECK_IMAGE
    .byte 'O','U','R','S','-','K','E','R','N','E','L'
#endif
.align 8
FUNCTION_NOCFI(_start)
    // Disable interrupts
    cli

    // Setup a temporary stack
    mov $PHYS(KERNEL_STACK_END), %rsp

    // Save the relocated_base of our main module 
    mov %rdi, PHYS(KERNEL_VIRT_BASE)

    // Save handoff
    mov %rsi, %r15

.Lzero_bss:
    movl $PHYS(__bss_start), %edi
    movl $PHYS(__bss_end), %ecx
    sub %edi, %ecx  // Compute the length of the bss in bytes.
    xor %eax, %eax
    rep stosb // while (ecx-- > 0) *edi++ = al;

.Lconstruct_page_table:
    // 1) Identity-maps the first 1GB of physical memory (VA [0, 1GB) = PA [0, 1GB)).

    // KERNEL_PGD[0] = virt_to_phys(KERNEL_PDP) | X86_KERNEL_PD_FLAGS

    movl $PHYS(KERNEL_PDP), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
    movl %eax, PHYS(KERNEL_PGD)

    // KERNEL_PDP[0] = virt_to_phys(KERNEL_PD) | X86_KERNEL_PD_FLAGS

    movl $PHYS(KERNEL_PD), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
    movl %eax, PHYS(KERNEL_PDP)

    // KERNEL_PGD[relocate(KRENEL_BASE)] = virt_to_phys(relocate(KERNEL_PDP_HIGH)) | X86_KERNEL_PD_FLAGS

    mov $PHYS(KERNEL_PDP_HIGH), %eax
    orl  $X86_KERNEL_PD_FLAGS,  %eax
    install_relocated_pte KERNEL_PGD, X86_PML4_SHIFT, %eax

    // KERNEL_PDP_HIGH[relocate(KRENEL_BASE)] = virt_to_phys(relocate(KERNEL_PD)) | X86_KERNEL_PD_FLAGS

    movl $PHYS(KERNEL_PD), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
    install_relocated_pte KERNEL_PDP_HIGH, X86_PDP_SHIFT, %eax

    // Start to map
    movl $PHYS(KERNEL_PD), %esi
    movl $512, %ecx
    xor  %eax, %eax
0:
    mov  %eax, %ebx
    shll $21,  %ebx     // The low 21 bits of an address aligned to 2MB is zero 
    orl  $X86_KERNEL_LPD_FLAGS, %ebx
    movl %ebx, (%esi)
    addl $8,%esi
    inc  %eax
    loop 0b

    // 2) Establish direct mapping, namely physmap.
    movl $PHYS(KERNEL_PHYSMAP_PD), %esi
    movl $NR_PHYSMAP_PTES,  %ecx
    xor  %eax, %eax
0:
    mov  %eax, %ebx
    shll $21,  %ebx // The low 21 bits of an address aligned to 2MB is zero absolutely
    orl  $X86_KERNEL_LPD_FLAGS, %ebx
    movl %ebx, (%esi)
    mov  %eax, %ebx
    shll $11,  %ebx     // Skip the remaining 11 bits of low 32 bits
    movl %ebx, 4(%esi)  // Save the high 32 bits of address.
    addl $8,%esi
    inc  %eax
    loop 0b

    //
    // void **esi = KERNEL_PDP_HIGH + (512 - KERNEL_ASPACE_SIZE / GB(1));
    // void *eax = &KERNEL_PHYSMAP_PD | X86_KERNEL_PD_FLAGS;
    // for (int i = 0; i < 64; ++i) {
    //     esi[i] = eax + i * 4096;
    // }
    //
    movl $PHYS(KERNEL_PDP_HIGH + PDP_HIGH_OFFSET * 8), %esi
    movl $NR_PHYSMAP_PDS, %ecx
    movl $PHYS(KERNEL_PHYSMAP_PD), %eax
    orl  $X86_KERNEL_PD_FLAGS, %eax
0:
    movl %eax, (%esi)
    add  $8,    %esi
    addl $4096, %eax
    loop 0b 

.Linstall_page_table:
    // Disable global pages before installing the new root page table 
    // to ensure that any prior global TLB entries are flushed.
    mov   %cr4, %rax
    and   $(~X86_CR4_PGE), %rax
    mov   %rax, %cr4

    // Load the physical pointer to the root page table.
    mov $PHYS(KERNEL_PGD), %rax
    mov %rax, %cr3

    // Now the new root has been installed, re-enable global pages.
    mov   %cr4, %rax
    or    $(X86_CR4_PGE), %rax
    mov   %rax, %cr4

    push  $X86_GDT_KERNEL_CODE64
    mov   $PHYS(prepare_start_kernel), %rax
    addq  PHYS(KERNEL_VIRT_BASE), %rax
    pushq %rax
    lretq

.text
// Now we is on the high address. For ASLR, the code bottom is required being PIC.
prepare_start_kernel:
    // Zero our kernel segment data registers
    xor %eax, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    // Page table has been changed, so we must reload the data before
    // to rectify their address.
    lea KERNEL_STACK_END(%rip), %rsp

    lgdt TEMP_GDTR(%rip)

    // Call our main module
    mov %r15, %rdi  // Pass handoff
    call start_kernel

idle:
    hlt
    pause
    jmp idle

.bss
.align 16
DATA(KERNEL_STACK)
    .skip KB(2)
DATA_END(KERNEL_STACK)
DATA(KERNEL_STACK_END)

.global MAIN_ENTRY
MAIN_ENTRY = _start;
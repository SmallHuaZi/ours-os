#include "multiboot2.hpp"
#include <ours/arch/asm.hpp>
#include <ours/macro_abi.hpp>

.equ MBH_SIZE,  MULTIBOOT_HEADER_END - MULTIBOOT_HEADER
.equ MBH_CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + MBH_SIZE)  

.section .boot.header, "a", @progbits
    jmp     _start

.align 4
MULTIBOOT_HEADER:
    .long MULTIBOOT2_HEADER_MAGIC       // Magic
    .long MULTIBOOT_ARCHITECTURE_I386   // ISA: I386
    .long MBH_SIZE                      // Header length
    .long MBH_CHECKSUM                  // Check sum
    
.align 4
FINAL_TAG:
    // End tag
    .short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long FINAL_TAG_END - FINAL_TAG 
FINAL_TAG_END:
MULTIBOOT_HEADER_END:

.code32
.text
.align 8
FUNCTION_NOCFI(_start)
    cmp $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
    jne bad_multiboot_magic

    // TODO(SmallHuaZi) This ia a workaround for the transition to relocatable phys.kernel.
    lea  ENTRY_POINT, %edi
    mov %ebx, %esi
    call ENTRY_POINT

bad_multiboot_magic:
    ud2
    jmp bad_multiboot_magic

.global PHYS_ENTRY
PHYS_ENTRY = _start;

.global ENTRY_POINT
ENTRY_POINT = x86_start
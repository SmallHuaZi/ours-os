#define ASM
#include <multiboot/multiboot2.hpp>

    .extern x86_start

    .equ MBH_SIZE,  MULTIBOOT_HEADER_END - MULTIBOOT_HEADER
    .equ MBH_CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + MBH_SIZE)  

    .section .multiboot
    .align 8
MULTIBOOT_HEADER:
    // Magic
    .long MULTIBOOT2_HEADER_MAGIC
    // ISA: I386
    .long MULTIBOOT_ARCHITECTURE_I386
    // Header length
    .long MBH_SIZE 
    // Check sum
    .long MBH_CHECKSUM 

//     .align 8
// FRAMEBUFFER_TAG_START: 
//     // Frame buffer tag
//     .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
//     .short MULTIBOOT_HEADER_TAG_OPTIONAL
//     .long FRAMEBUFFER_TAG_END - FRAMEBUFFER_TAG_START
//     .long 720 
//     .long 480 
//     .long 0 
// FRAMEBUFFER_TAG_END:

    .align 8
FINAL_TAG:
    // End tag
    .short MULTIBOOT_HEADER_TAG_END 
    .short 0
    .long FINAL_TAG_END - FINAL_TAG 
FINAL_TAG_END:
MULTIBOOT_HEADER_END:
// Work around.
//----------------------------
//      Return Address
//----------------------------
// pushfl
// pushw    %fs
// pushw    %gs
// pushal
//----------------------------
//          %edi
//----------------------------
//          %esi
//----------------------------
//          %ebp
//----------------------------
//          %esp
//---------------------------- 
//          %ebx
//---------------------------- 
//          %edx
//---------------------------- 
//          %ecx
//---------------------------- 
//          %eax
//---------------------------- 
//          %gs
//---------------------------- 
//          %fs
//---------------------------- 
//          %eflags
//----------------------------   <-----------
// subw    $44,    %sp                      | 
// movw    %dx,    %si                      | 
// movw    %sp,    %di                      | 
// movw    $11,    %cx                      | 
// rep movsd                                | 
//---------------------------               |
//          %edi (in/out)                   |
//---------------------------               |
//          %esi (in/out)                   |
//---------------------------               |
//          %ebp (in/out)                   |
//---------------------------               |
//          %esp (in/out)                   |
//---------------------------               |
//          %ebx (in/out)                   |
//---------------------------               |
//          %edx (in/out)                   |
//---------------------------               |
//          %ecx (in/out)                   |
//---------------------------               |
//          %eax (in/out)                   |
//---------------------------               |
//          %gs (in/out)                    |
//---------------------------               |
//          %fs (in/out)                    |
//---------------------------               |
//          %es (in/out)                    |
//---------------------------               |
//          %ds (in/out)                    | 
//---------------------------               |
//          %eflags (in/out)                |
//---------------------------    <-------------------
// popal                                    |       | 
// popw    %gs                              |       | 
// popw    %fs                              |       | 
// popw    %es                              |       | 
// popw    %ds                              |       | 
// popfl                                    |       | 
//----------------------------              |       | 
// ------------------------------------------       |
//----------------------------                      |
// pushfl                                           |
// pushw   %ds                                      |
// pushw   %es                                      |
// pushw   %fs                                      |
// pushw   %gs                                      |
// pushal                                           |
//----------------------------                      |
//---------------------------------------------------
//---------------------------- 
//---------------------------- 
//---------------------------- 
//---------------------------- 
//---------------------------- 

#include <navigator/symbol.hpp>

IMPORT_SYMBOL(switch_to_realmode)
IMPORT_SYMBOL(switch_to_protmode)

    .code16
    .section    ".text", "ax"

/// cpp-signature: extern "C" interrupt_early_protmode(u8 id, X86Registers *regs)
/// @brief Provide BIOS interrupt call in protected mode.
/// @param %ax  Bios interrupt id
/// @param %dx  in/out context
GLOBAL_FUNCTION(interrupt_early_protmode)
    // Return immediately if no the in/out context.
    testw   %dx,    %dx
    jz      1f

    // the function `switch_to_realmode` will modify %eax and %edx,
    // so we have to save temporarily them to stack.
    pushw   %ax
    pushw   %dx
    calll   switch_to_realmode

    // Here, we are finally safe! then we do the real work.
    popw    %dx
    popw    %ax
    calll   interrupt_early_realmode

    // Switch to the real mode.
    calll   switch_to_protmode
1:
    retl
GLOBAL_FUNCTION_END(interrupt_early_protmode)


/// cpp-signature: extern "C" interrupt_early_realmode(u8 id, ArchInterruptState *regs)
//! @brief Provide BIOS interrupt call in real mode.
//! @param %ax
//! @param %dx
GLOBAL_FUNCTION(interrupt_early_realmode)
     // Return immediately if no the in/out context.
    testw   %dx,    %dx
    jz      2f

    // Set interrupt number.
    movw    %ax,    %cs:no

    // Save temporarily the address of the input context.
    pushw   %dx

    // Save interrupt context.
    pushfl
    pushw   %fs
    pushw   %gs
    pushal

    // We need a sapce of 44 bytes to create buffer for BiosRegister.
    subw    $44,    %sp

    // Push full state, the BiosRegister in/outed via %dx, to the stack. 
    movw    %dx,    %si
    movw    %sp,    %di
    movw    $11,    %cx
    rep movsd

    // Pop full state, the BiosRegister in/outed via %dx, from the stack.
    popal
    popw    %gs
    popw    %fs
    popw    %es
    popw    %ds
    popfl

    // int imm8.
    .byte 0xCD
no: .byte 0

    // Push full state to the stack.
    pushfl
    pushw   %ds
    pushw   %es
    pushw   %fs
    pushw   %gs
    pushal

    // Ok, let us get the address of the input context from stack.
    movw    44(%esp),    %dx

    cld
    movw    %sp,    %si
    movw    %dx,    %di
    movw    $11,    %cx
    rep movsd

    // Recycle the buffer.
    addw    $48,    %sp

    // Restore interrupt context.
    pushal
    pushw   %gs
    pushw   %fs
    pushfl
2: 
    retl
GLOBAL_FUNCTION_END(interrupt_early_realmode)
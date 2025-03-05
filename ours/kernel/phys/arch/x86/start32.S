#include <ours/asm.hpp>
#include <ours/marco_abi.hpp>

// [[no_return]] auto x86_start(LegacyBootParam %esi) -> void 
FUNCTION_NOCFI(x86_start)
    cli

    // Clear .bss section. It assumes it's aligned to 4.
    cld
    xor %eax, %eax
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    sub %edi, %ecx
    shr $2, %ecx
    rep stosl

    // Load ealry stack
    mov $EARLY_STACK_END, %esp

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
    movl $EARLY_GDT, -4(%esp)
    movw $(EARLY_GDT_END - EARLY_GDT), -6(%esp)
    lgdt -6(%esp)

    // Reload CS 
    ljmp $(1 << 3), $0f

0:
    mov $(2 << 3), %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    xor %eax, %eax
    mov %ax, %fs

    xor %ebp, %ebp

#ifdef OURS_CONFIG_TARGET_64BIT
    // If target is 64 bit, we should push an fake 32bit value as the high bits of the loader parameter
    push $0     // High 32 bits
#endif
    push %esi   // Low 32 bits
    call phys_main

idle:
    cli
    hlt
    jmp idle 

.bss
.align 8
LOCAL_DATA(EARLY_STACK)
    .skip KB(2)
DATA_END(EARLY_STACK)
LOCAL_DATA(EARLY_STACK_END)

.global OBIMAGE
OBIMAGE = x86_start

.data
.align 8
DATA(EARLY_GDT)
    // Null entry
    .int    0
    .int    0

    /* CODE_SELECTOR */
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b10011010       /* P(1) DPL(00) S(1) 1 C(0) R(1) A(0) */
    .byte  0b11001111       /* G(1) D(1) 0 0 limit 19:16 */
    .byte  0x0              /* base 31:24 */

    // Data selector
    .short 0xffff           /* limit 15:00 */
    .short 0x0000           /* base 15:00 */
    .byte  0x00             /* base 23:16 */
    .byte  0b10010010       /* P(1) DPL(00) S(1) 0 E(0) W(1) A(0) */
    .byte  0b11001111       /* G(1) B(1) 0 0 limit 19:16 */
    .byte  0x0              /* base 31:24 */

    // Code 64 selector
    .int    0
    .int    0

    // Gs selector
    .int    0
    .int    0
DATA_END(EARLY_GDT)
DATA(EARLY_GDT_END)
#include <ours/arch/asm.hpp>
#include <ours/mem/cfg.hpp>

.extern interrupt_common

// This is called 256 times in a row, with isr.current set to the iteration
// count starting at 0.  This is inside the definition for the ISR table,
// below (equivalent to being inside an .object ... .end_object pair,
// though those macros are not actually used because they don't allow
// nesting .function ... .end_function inside).  So the ambient section
// state is building up the table, and the macro defines a function using
// pushsection/popsection before adding its pointer to the table.
.macro isr.dispatch.define name
    // Make the whole set of macro-generated functions be cache-line aligned
    // collectively.
    .pushsection .text.isr.table, "ax", %progbits
    .ifeq isr.current
        .balign 64
    .endif
    .function \name, cfi=custom, nosection=nosection
        // Set CFI for an interrupt frame.
        .cfi_signal_frame
        isr.current.has_error = isr.current == 8 || \
                                (isr.current >= 10 && isr.current <= 14) || \
                                isr.current == 17

        .cfi_def_cfa %rsp, (8 * (5 + isr.current.has_error))
        .cfi_offset %rip, -(5 * 8)
        // Mark each reg as having the same value as from the "calling" frame.
        // This is the default state for callee-saved registers, but for
        // completeness sake we do this for all of them.
        all_cfi_same_value

        // Clear the AC flag to prevent ring 0 from performing data accesses to
        // ring 3 if SMAP is available.  If it was set, it will get restored by
        // iretd.  DO NOT REMOVE THIS CLAC, code in idt.c assumes it is here.
        // It MUST be the first instruction of this function.
        clac
     .if !isr.current.has_error
        // Fill in the error code not pushed by the hardware.
        push_val $0
     .endif
        // Fill in the interrupt number.
        push_val $isr.current
        jmp interrupt_common

        // Should never go back here.
        int3
    .end_function
    .popsection

    // Now we're back in the section building up the table.
    .quad \name
    .quad 0
.endm

.macro isr.dispatch.define.next
    // The name is generated with an arbitrary number and is not actually
    // meaningful at all.  But there's no way to get isr.current into a
    // symbol name here.
    isr.dispatch.define isr.dispatch.\@.is.not.the.isr.number
    isr.current = isr.current + 1
.endm

.section .data.rel.ro.idt, "aw", %progbits
.balign PAGE_SIZE
.label g_idt, global, object
    isr.current = 0
    .rept 256
        isr.dispatch.define.next
    .endr
.size g_idt, . - g_idt
#include <ours/arch/asm.hpp>

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
    .pushsection .text.isr.dispatch, "ax", %progbits
    .ifeq isr.current
        .balign 64
    .endif
    .function \name, cfi=custom, nosection=nosection
        // Set CFI for an interrupt frame.
        .cfi_signal_frame
        isr.current.has_error = isr.current == 8 || (isr.current >= 10 && isr.current <= 14) || isr.current == 17
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
    .end_function
    .popsection

    // Now we're back in the section building up the table.
    .quad \name
.endm

.macro isr.dispatch.define.next
    // The name is generated with an arbitrary number and is not actually
    // meaningful at all.  But there's no way to get isr.current into a
    // symbol name here.
    isr.dispatch.define isr.dispatch.\@.is.not.the.isr.number
    isr.current = isr.current + 1
.endm

// This defines the table with pointers to all the macro-generated functions.
// This does the same as .object, but .function can't be nested inside .object.
.pushsection .data.rel.ro.isr_table, "aw", %progbits
.balign 8
.label ISR_TABLE, global, object
    isr.current = 0
    .rept 256
        isr.dispatch.define.next
    .endr
.size ISR_TABLE, . - ISR_TABLE
.popsection

// This is the real function all the macro-generated functions tail-call into.
.function interrupt_common, global, align=64, cfi=custom
    // Set CFI for an interrupt frame, with all the words pushed by the hardware
    // and the macro-generated functions together.
    .cfi_signal_frame
    .cfi_def_cfa %rsp, 7 * 8
    .cfi_offset %rip, -(5 * 8)

    // Mark each reg as having the same value as from the "calling" frame.
    // This is the default state for callee-saved registers, but for completeness
    // sake we do this for all of them.
    all_cfi_same_value

    // Clear the direction flag.  Without this, uses of string
    // instructions, e.g. REP MOVS in memcpy() or inlined by the compiler,
    // can go wrong and copy in the wrong direction, since this code may
    // assume that the direction flag is unset.
    cld

    // Later on we're going to calculate the percpu pointer using dead reckoning
    // from %rsp so keep track of what all has been pushed onto the stack.
    //
    // At this point the stack contains 7 qwords.  5 or 6 pushed by the interrupt
    // and 2 or 1 pushed by the stub.
    save_general_registers

    // Zero general-purpose registers to constrain speculative execution with
    // potentially-hostile register values; the values may have come from user
    // mode and could be consumed by in-kernel speculative execution.
    // Zeroing of ebp is delayed till we check precisely if this was a user fault
    // so that kernel faults can have correct stack frames for backtraces.
    zero_general_registers

    // At this point the stack contains a complete Iframe.

    // Pass the Iframe in %rdi.
    movq %rsp, %rdi     

    // Check to see if we came from user space by testing the CPL in the
    // %cs selector that was saved on the stack automatically.  Check for != 0.
    testb $3, X86_IFRAME_OFFSET_CS(%rsp)
    jz 1f

    // Perform the last zero from the previous block now that we know this is a
    // user fault and we do not need the stack frame.
    xorl %ebp, %ebp
    // Swap %gs.base to kernel space.
    swapgs
1:
    // Mitigates the swapgs bug. See <arch/code-patches/case-id.h>.
    // .code_patching.start CASE_ID_SWAPGS_MITIGATION
    lfence
    // .code_patching.end

    call arch_handle_exception

    // A label to assist gdb's backtracing through kernel exceptions.
    // When gdb sees this as the return address it knows it can fetch
    // iframe_t from $rsp. See scripts/zircon.elf-gdb.py.
    .label interrupt_common_iframe_set_up_for_debugger

    // Label exists only so a test can verify the code patching result.
    .label interrupt_non_nmi_maybe_mds_buff_overwrite, global

    // Mitigates MDS/TAA bugs. See <arch/code-patches/case-id.h>
    // .code_patching.start CASE_ID_MDS_TAA_MITIGATION
    // call mds_buff_overwrite
    // .code_patching.end

    // Check if we're returning to user space as per before.
    testb $3, X86_IFRAME_OFFSET_CS(%rsp)
    jz    1f
    // Swap %gs.base back to user space.
    swapgs
1:
    // Mitigates the swapgs bug. See <arch/code-patches/case-id.h>.
    // .code_patching.start CASE_ID_SWAPGS_MITIGATION
    lfence
    // .code_patching.end

.Lcommon_return:
    // Restore general purpose registers.
    restore_general_registers

    // Drop vector number and error code.
    add_to_sp 16

    iretq

.end_function
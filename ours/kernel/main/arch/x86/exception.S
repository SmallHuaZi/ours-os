#include <arch/asm.hpp>

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
    ALL_CFI_SAME_VALUE

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

    // At this point the stack contains a complete iframe_t.

    movq %rsp, %rdi     // Pass the iframe in %rdi.

    // There are two main paths through this function.  One path is for NMIs.  The
    // other is for all other interrupts (non_nmi).  Both share a common return
    // path.
    //
    // TODO(maniscalco): Refactor this function to eliminate the compare and branch
    // without duplicating logic that's common to the NMI and non-NMI paths.

    // Was this an NMI?
    cmpq $2, X86_IFRAME_OFFSET_VECTOR(%rsp)
    je .Lnmi

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
    .code_patching.start CASE_ID_SWAPGS_MITIGATION
    lfence
    .code_patching.end

    call arch_handle_exception

    // A label to assist gdb's backtracing through kernel exceptions.
    // When gdb sees this as the return address it knows it can fetch
    // iframe_t from $rsp. See scripts/zircon.elf-gdb.py.
.l  abel interrupt_common_iframe_set_up_for_debugger

    // Label exists only so a test can verify the code patching result.
.l  abel interrupt_non_nmi_maybe_mds_buff_overwrite, global
    // Mitigates MDS/TAA bugs. See <arch/code-patches/case-id.h>
    .code_patching.start CASE_ID_MDS_TAA_MITIGATION
    call mds_buff_overwrite
    .code_patching.end

    // Check if we're returning to user space as per before.
    testb $3, X86_IFRAME_OFFSET_CS(%rsp)
    jz    1f
    // Swap %gs.base back to user space.
    swapgs
1:
    // Mitigates the swapgs bug. See <arch/code-patches/case-id.h>.
    .code_patching.start CASE_ID_SWAPGS_MITIGATION
    lfence
    .code_patching.end

.Lcommon_return:
    // Restore general purpose registers.
    restore_general_registers

    // Drop vector number and error code.
    add_to_sp 16

    iretq
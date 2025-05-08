#include <ours/arch/asm.hpp>

/// cpp-signature: NO_MANGLE auto x86_enter_uspace(arch::IrqFrame *t) -> void
/// Restoring user's status and changing privileges.
.function x86_enter_uspace, global, align=64, cfi=custom
    // push a fake 64bit interrupt stack frame and iret to it
    push_val X86_IRQFRAME_OFFSET_USER_SS(%rdi) // ss
    push_val X86_IRQFRAME_OFFSET_USER_SP(%rdi) // sp
    push_val X86_IRQFRAME_OFFSET_FLAGS(%rdi)   // rflags
    push_val X86_IRQFRAME_OFFSET_CS(%rdi)      // cs
    push_val X86_IRQFRAME_OFFSET_IP(%rdi)      // pc

    // Copy the remaining register state from |t|.
    movq X86_IRQFRAME_OFFSET_RAX(%rdi), %rax
    movq X86_IRQFRAME_OFFSET_RBX(%rdi), %rbx
    movq X86_IRQFRAME_OFFSET_RCX(%rdi), %rcx
    movq X86_IRQFRAME_OFFSET_RDX(%rdi), %rdx
    movq X86_IRQFRAME_OFFSET_RBP(%rdi), %rbp
    movq X86_IRQFRAME_OFFSET_RSI(%rdi), %rsi
    // Setting rdi deferred to last, we're still using it.
    movq X86_IRQFRAME_OFFSET_R8(%rdi), %r8
    movq X86_IRQFRAME_OFFSET_R9(%rdi), %r9
    movq X86_IRQFRAME_OFFSET_R10(%rdi), %r10
    movq X86_IRQFRAME_OFFSET_R11(%rdi), %r11
    movq X86_IRQFRAME_OFFSET_R12(%rdi), %r12
    movq X86_IRQFRAME_OFFSET_R13(%rdi), %r13
    movq X86_IRQFRAME_OFFSET_R14(%rdi), %r14
    movq X86_IRQFRAME_OFFSET_R15(%rdi), %r15
    movq X86_IRQFRAME_OFFSET_RDI(%rdi), %rdi

    swapgs
    iret
.end_function
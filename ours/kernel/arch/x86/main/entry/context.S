#include <ours/arch/asm.hpp>

// BUG(SmallHuaZi)
#define OURS_THREAD_STACK_OF

/// NO_MANGLE auto x86_switch_context(VirtAddr *oldsp, VirtAddr newsp) -> void
/// %rdi: oldsp 
/// %rsi: newsp
FUNCTION(x86_switch_context)
    // Save callee-saved registers
    push_reg %rbx
    push_reg %rbp
    push_reg %r12
    push_reg %r13
    push_reg %r14
    push_reg %r15
    pushf

    // Second, switching kernel stack from `prev's` to `next's`
    mov %rsp, (%rdi)
    mov %rsi, %rsp

    // Restore callee-saved registers
    popf
    pop_reg %r15
    pop_reg %r14
    pop_reg %r13
    pop_reg %r12
    pop_reg %rbp
    pop_reg %rbx

    // Finally, return to cpp-code
    ret
    int3
FUNCTION_END(x86_switch_context)


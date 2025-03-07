#include <ours/asm.hpp>

// BUG(SmallHuaZi)
#define OURS_THREAD_STACK_OF

// arch_switch_context -> x86_prepare_context_switch_asm -> x86_switch_context
//                     -> ...
//                     -> 

/// NO_MANGLE auto x86_switch_context_asm(Thread *prev, Thread *next) -> void
/// %eax: prev
/// %ebx: next 
FUNCTION(x86_switch_context_asm)
    // First, storing `prev's` state
    push   %rbp
    push   %rbx
    push   %rdi
    push   %rsi
    pushf

    // Second, switching stack from `prev's` to `next's`
    movl    %esp, OURS_THREAD_STACK_OF(%eax)
    movl    OURS_THREAD_STACK_OF(%ebx), %ebx

#ifdef THREAD_STACK_SAFETY
    //
#endif

    // Third, restoring `next's` state 
    popf
    pop    %rsi
    pop    %rdi
    pop    %rbx
    pop    %rbp

    // Finally, call cpp-code 
    jmp x86_switch_context
FUNCTION_END(x86_switch_context_asm)


/// cpp-signature: NO_MANGLE auto x86_switch_to_user_mode(arch::IrqFrame *t) -> void
/// Restoring user's status and changing privileges.
FUNCTION(x86_switch_to_user_mode)
    // Switching back the address space of user(CR3)

    // Restoring saved registers
    iret
FUNCTION_END(x86_switch_to_user_mode)
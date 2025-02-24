// arch_switch_context -> x86_prepare_context_switch_asm -> x86_switch_context
//                     -> ...
//                     -> 

/// cpp-signature: extern "C" void x86_switch_context_asm(Thread *prev, Thread *next)
/// %eax: prev
/// %ebx: next 
GLOBAL_FUNCTION(x86_switch_context_asm)
    // First, storing `prev's` state
    pushl   %ebp
    pushl   %ebx
    pushl   %edi
    pushl   %esi
    pushfl

    // Second, switching stack from `prev's` to `next's`
    movl    %esp, OURS_THREAD_STACK_OF(%eax)
    movl    OURS_THREAD_STACK_OF(%ebx), %ebx

#if OURS_CFG(THREAD_STACK_SAFETY)
    //
#endif

    // Third, restoring `next's` state 
    popfl
    popl    %esi
    popl    %edi
    popl    %ebx
    popl    %ebp

    // Finally, call cpp-code 
    jmp x86_switch_context
GLOBAL_FUNCTION_END(x86_switch_context_asm)


/// cpp-signature: extern "C" void x86_switch_to_user_mode(Thread *prev, Thread *next)
/// Restoring user's status and changing privileges.
GLOBAL_FUNCTION(x86_switch_to_user_mode)
    // Switching back the address space of user(CR3)

    // Restoring saved registers
    iret
GLOBAL_FUNCTION_END(x86_switch_to_user_mode)
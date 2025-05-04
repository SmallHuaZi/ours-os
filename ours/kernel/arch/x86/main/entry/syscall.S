#include <ours/arch/asm.hpp>

/// Intel® 64 and IA-32 Architectures Software Developer's Manual, Volume 2B:
///
/// SYSCALL saves the RIP of the instruction following SYSCALL to RCX and loads a new
/// RIP from the IA32_LSTAR (64-bit mode). Upon return, SYSRET copies the value
/// saved in RCX to the RIP. 
///
/// SYSCALL saves RFLAGS (lower 32 bit only) in R11. It then masks RFLAGS with an
/// OS-defined value using the IA32_FMASK (MSR C000_0084). The actual mask value
/// used by the OS is the complement of the value written to the IA32_FMASK MSR.
/// None of the bits in RFLAGS are automatically cleared (except for RF). SYSRET
/// restores RFLAGS from R11 (the lower 32 bits only).
///
/// Software should not alter the CS or SS descriptors in a manner that violates the
/// following assumptions made by SYSCALL/SYSRET:
///     1. The CS and SS base and limit remain the same for all processes, including the
///     operating system (the base is 0H and the limit is 0FFFFFFFFH).
///     2. The CS of the SYSCALL target has a privilege level of 0.
///     3. The CS of the SYSRET target has a privilege level of 3.
///
/// SYSCALL/SYSRET do not check for violations of these assumptions.
/// IF (CS.L ≠ 1 ) or (IA32_EFER.LMA ≠ 1) or (IA32_EFER.SCE ≠ 1) 
/// Namely Not in 64-Bit Mode or SYSCALL/SYSRET not enabled in IA32_EFER *)
///     THEN #UD; 
/// FI;
/// RCX ← RIP;
/// RIP ← LSTAR_MSR;
/// R11 ← EFLAGS;
/// EFLAGS ← (EFLAGS MASKED BY IA32_FMASK);
/// CPL ← 0;
/// CS(SEL) ← IA32_STAR_MSR[47:32];
/// CS(DPL) ← 0;
/// CS(BASE) ← 0;
/// CS(LIMIT) ← 0xFFFFF;
/// CS(GRANULAR) ← 1;
/// SS(SEL) ← IA32_STAR_MSR[47:32] + 8;
/// SS(DPL) ← 0;
/// SS(BASE) ← 0;
/// SS(LIMIT) ← 0xFFFFF;
/// SS(GRANULAR) ← 1;
///
/// On syscall,
///     CS = IA32_STAR_MSR[47:32]
///     SS = IA32_STAR_MSR[47:32] + 8
.function x86_handle_syscall, global, align=64, cfi=custom
    // Save user stack
    // Load kernel stack

    push_val %r11  // RFLAGS
    push_val %rcx  // RIP
    push_val %r15
    push_val %r14
    push_val %r13
    push_val %r12
    push_val $0    // R11 was trashed by the syscall instruction.
    push_val %r10
    push_val %r9
    push_val %r8
    push_val %rbp
    push_val %rdi
    push_val %rsi
    push_val %rdx
    push_val $0    // RCX was trashed by the syscall instruction.
    push_val %rbx
    push_val %rax

.end_function

/// SYSCALL saves the RIP of the instruction following the SYSCALL into RCX and loads
/// the new RIP from the LSTAR (64-bit mode only). Upon return, SYSRET copies the
/// value saved in RCX to the RIP.
/// In a return to 64-bit mode using Osize 64, SYSRET sets the CS selector value to MSR
/// IA32_STAR[63:48] +16. The SS is set to IA32_STAR[63:48] + 8.
/// SYSRET transfer control to compatibility mode using Osize 32. The CS selector value
/// is set to MSR IA32_STAR[63:48]. The SS is set to IA32_STAR[63:48] + 8.
/// It is the responsibility of the OS to keep descriptors in the GDT/LDT that correspond
/// to selectors loaded by SYSCALL/SYSRET consistent with the base, limit and attribute
/// values forced by the these instructions.
/// Software should not alter the CS or SS descriptors in a manner that violates the
/// following assumptions made by SYSCALL/SYSRET:
/// • CS and SS base and limit remain the same for all processes, including the
/// operating system.
/// • CS of the SYSCALL target has a privilege level of 0.
/// • CS of the SYSRET target has a privilege level of 3.
/// SYSCALL/SYSRET do not check for violations of these assumptions.
/// IF (CS.L ≠ 1 ) or (IA32_EFER.LMA ≠ 1) or (IA32_EFER.SCE ≠ 1)
/// (* Not in 64-Bit Mode or SYSCALL/SYSRET not enabled in IA32_EFER *)
///     THEN #UD; 
/// FI;
/// IF (CPL ≠ 0)
///     THEN #GP(0); 
/// FI;
/// IF (RCX ≠ CANONICAL_ADDRESS)
///     THEN #GP(0);
/// FI;
/// IF (OPERAND_SIZE = 64)
///     THEN (* Return to 64-Bit Mode *)
///     EFLAGS ← R11;
///     CPL ← 0x3;
///     CS(SEL) ← IA32_STAR[63:48] + 16;
///     CS(PL) ← 0x3;
///     SS(SEL) ← IA32_STAR[63:48] + 8;
///     SS(PL) ← 0x3;
///     RIP ← RCX;
/// ELSE (* Return to Compatibility Mode *)
///     EFLAGS ← R11;
///     CPL ← 0x3;
///     CS(SEL) ← IA32_STAR[63:48];
///     CS(PL) ← 0x3;
///     SS(SEL) ← IA32_STAR[63:48] + 8;
///     SS(PL) ← 0x3;
///     EIP ← ECX;
/// FI;
///
/// On sysexit,
///     CS = IA32_STAR_MSR[63:48] + 16
///     SS = IA32_STAR_MSR[63:48] + 8
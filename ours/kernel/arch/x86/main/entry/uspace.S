#include <ours/arch/asm.hpp>

/// cpp-signature: NO_MANGLE auto x86_switch_to_user_mode(arch::IrqFrame *t) -> void
/// Restoring user's status and changing privileges.
.function x86_enter_uspace, global, align=64, cfi=custom
    // Switching back the address space of user(CR3)

    // Restoring saved registers
    iret
.end_function
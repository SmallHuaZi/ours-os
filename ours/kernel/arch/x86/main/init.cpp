#include <ours/arch/x86/init.hpp>
#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/mem/cfg.hpp>

namespace ours {
    NO_MANGLE INIT_CODE 
    auto init_arch_early() -> void
    {
        // Setup GDT early
        // Setup IDT early
        x86_setup_idt_early();
        // Setup MMU ealry
        x86_setup_mmu_early();
    }

    NO_MANGLE INIT_CODE 
    auto init_arch() -> void
    {
        x86_setup_idt();
        x86_setup_mmu();
    }

} // namespace ours
#include <ours/arch/x86/init.hpp>
#include <ours/arch/x86/idt.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/arch/x86/descriptor.hpp>

#include <ours/cpu-states.hpp>
#include <ours/arch/cpu.hpp>
#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/mem/cfg.hpp>

#include <ours/arch/aspace_layout.hpp>

namespace ours {
    NO_MANGLE INIT_CODE 
    auto init_arch_early() -> void {
        x86_init_idt_early();

        x86_init_percpu(0);

        x86_dump_gdt();
        
        x86_setup_mmu_early();

        set_current_cpu_online(true);
    }

    NO_MANGLE INIT_CODE 
    auto init_arch() -> void {
        x86_setup_idt();

        x86_setup_gdt();

        x86_setup_mmu();
    }

} // namespace ours
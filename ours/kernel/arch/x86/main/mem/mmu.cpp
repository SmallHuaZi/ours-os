#include <ours/arch/x86/init.hpp>
#include <ours/phys/arch_paging.hpp>

#include <arch/system.hpp>
#include <arch/tlb.hpp>

namespace ours {
namespace mem {
    NO_MANGLE PhysAddr g_pgd = 0;
}
    using mem::g_pgd;
    using arch::Cr3;

    auto x86_setup_mmu_early() -> void {
        g_pgd = Cr3::read().get<Cr3::PageTableAddress>();
    }

    auto x86_setup_mmu_percpu() -> void {

    }

    auto x86_setup_mmu() -> void {
        // Unmap low address.
        reinterpret_cast<arch::Pte *>(g_pgd)[0] = 0;
        arch::tlb_flush_all();

        x86_setup_mmu_percpu();
    }

} // namespace ours
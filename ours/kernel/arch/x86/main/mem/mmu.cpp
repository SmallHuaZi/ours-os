#include <ours/arch/x86/init.hpp>
#include <arch/system.hpp>
#include <arch/tlb.hpp>

namespace ours {
    using arch::Cr3;

    PhysAddr root_page_table;

    auto x86_setup_mmu_early() -> void {
        root_page_table = Cr3::read().get<Cr3::PageTableAddress>();
    }

    auto x86_setup_mmu_percpu() -> void {

    }

    auto x86_setup_mmu() -> void {
        // Unmap low address.
        reinterpret_cast<arch::Pte *>(root_page_table)[0] = 0;
        arch::tlb_flush_all();

        x86_setup_mmu_percpu();
    }

} // namespace ours
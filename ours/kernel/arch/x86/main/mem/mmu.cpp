#include <ours/arch/x86/init.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/phys/arch-paging.hpp>
#include <ours/mem/vmm.hpp>

#include <arch/macro/msr.hpp>
#include <arch/system.hpp>
#include <arch/tlb.hpp>
#include <arch/system.hpp>
#include <arch/x86/msr.hpp>
#include <arch/x86/cpuid-observer.hpp>

namespace ours {
namespace mem {
    NO_MANGLE {
        PhysAddr g_kernel_pgd = 0;
        u8 g_arch_phys_addr_bits = 32;
        u8 g_arch_virt_addr_bits = 48;
    }
}
    using arch::Cr3;
    using arch::Cr0;
    using arch::Cr4;
    using arch::MsrIo;
    using arch::MsrRegAddr;

    auto x86_tlb_global_invalidate() -> void {
        if (x86_has_feature(CpuFeatureType::InvPcid)) {
            return arch::x86_invpcid_all();
        }

        arch::tlb_invalidate_all();
    }

    auto x86_setup_mmu_percpu() -> void {
        Cr0::read().set<Cr0::Wp>(1)  // Enable Write protect.
                   .set<Cr0::Nw>(0)  // Disable no-write-through.
                   .set<Cr0::Cd>(0)  // Disable cache-disable.
                   .write();

        Cr4::read().set<Cr4::Smep>(x86_has_feature(CpuFeatureType::Smep))   // Unable to execute user code.
                   .set<Cr4::Smap>(x86_has_feature(CpuFeatureType::Smap))   // Unable to read user data.
                   .set<Cr4::Pcide>(x86_has_feature(CpuFeatureType::Pcid))  // Enable process context identifier
                   .set<Cr4::Pge>(1) // Make kernel resident pages do not be refreshed out of TLB.
#if PAGING_LEVEL == 5
                   .set<Cr4::La57>(x86_has_feature(CpuFeatureType::La57))
#endif
                   .write();
        
        auto shadow = MsrIo::read<usize>(MsrRegAddr::IA32Efer);
        shadow |= X86_EFER_NXE;
        MsrIo::write(MsrRegAddr::IA32Efer, shadow);
    }

    auto x86_setup_mmu_early() -> void {
        mem::g_kernel_pgd = Cr3::read().get<Cr3::PageTableAddress>();

        // Tear down the transient 1:1 identity mappings established by `PhysBoot`
        // during early boot process, and permit low-memory regions to temporarily 
        // utilize the PhysMap virtual addressing scheme until full memory management
        // infrastructure is operational.
        auto pgd = reinterpret_cast<arch::PteVal *>(mem::g_kernel_pgd);
        pgd[0] = pgd[511];
        x86_tlb_global_invalidate();

        x86_setup_mmu_percpu();

        dispatch_cpuid(arch::AddrSizeCpuIdObserver(
            mem::g_arch_phys_addr_bits, 
            mem::g_arch_virt_addr_bits)
        );
#if PAGING_LEVEL == 5
        mem::g_arch_virt_addr_bits = 57;
#endif
    }

    auto x86_setup_mmu() -> void {
        // Unmap low address.
        reinterpret_cast<arch::PteVal *>(mem::g_kernel_pgd)[0] = 0;
        x86_tlb_global_invalidate();
    }

} // namespace ours
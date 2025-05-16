#include <ours/arch/x86/init.hpp>
#include <ours/arch/x86/idt.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/apic.hpp>
#include <ours/arch/x86/entry.hpp>

#include <ours/cpu-states.hpp>
#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/cpu-local.hpp>
#include <ours/mem/cfg.hpp>

#include <ours/arch/aspace_layout.hpp>

#include <arch/halt.hpp>
#include <arch/system.hpp>
#include <arch/macro/system.hpp>

namespace ours {
    INIT_CODE
    static auto x86_enable_syscall() -> void {
        using namespace arch;
        
        // Enable system call. It is required by Intel.
        auto efer = MsrIo::read<usize>(MsrRegAddr::IA32Efer);
        efer |= X86_EFER_SCE; // Enable long mode and long mode active
        MsrIo::write(MsrRegAddr::IA32Efer, efer);

        /// On syscall,
        ///     CS = IA32_STAR_MSR[47:32]
        ///     SS = IA32_STAR_MSR[47:32] + 8
        /// On sysexit,
        ///     CS = IA32_STAR_MSR[63:48] + 16
        ///     SS = IA32_STAR_MSR[63:48] + 8
        CXX11_CONSTEXPR
        auto const kCsAndSs((usize(X86_GDT_KERNEL_CODE64) << 32) | (usize(X86_GDT_USER_CODE32) << 48));
        MsrIo::write(MsrRegAddr::IA32Star, kCsAndSs);

        // Set system call entry point to x86_syscall
        MsrIo::write(MsrRegAddr::IA32Lstar, &x86_handle_syscall);

        // SYSCALL also saves RFLAGS into R11 and then masks RFLAGS using the 
        // IA32_FMASK MSR (MSR address C0000084H); specifically, the processor 
        // clears in RFLAGS every bit corresponding to a bit that is set in the 
        // IA32_FMASK MSR.
        //
        // 1. Disable nested interrupt.
        // 2. Escalates IO privilege level(U->K).
        // 3. Clear those unused status bits.
        CXX11_CONSTEXPR
        usize const kDisabledFlagBitMask {X86_FLAGS_NT | X86_FLAGS_IOPL_MASK | X86_FLAGS_STATUS_MASK};
        MsrIo::write(MsrRegAddr::IA32Fmask, kDisabledFlagBitMask);
    }

    INIT_CODE
    auto x86_init_percpu(CpuNum cpunum) -> void {
        using arch::Cr4;

        x86_init_feature_percpu(cpunum);

        // GDT
        x86_load_gdt();

        // TSS
        x86_setup_tss_percpu(cpunum);

        // IDT
        x86_load_idt();

        Cr4::read().set<Cr4::FsGsBase>(x86_has_feature(CpuFeatureType::FsGsBase)) // Enable FS/GS_BASE 
                   .set<Cr4::Umip>(x86_has_feature(CpuFeatureType::Umip)) // User mode instruction preventation
                   .write();

        x86_enable_syscall();

        x86_init_mmu_percpu();
    }

    NO_MANGLE INIT_CODE 
    auto init_arch_early() -> void {
        x86_init_idt_early();

        x86_init_percpu(0);

        x86_init_mmu_early();

        set_current_cpu_online(true);
    }

    NO_MANGLE INIT_CODE 
    auto init_arch() -> void {
        x86_setup_idt();

        x86_setup_gdt();

        x86_setup_mmu();
    }

} // namespace ours
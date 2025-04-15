#include <ours/arch/cpu.hpp>
#include <ours/arch/x86/feature.hpp>
#include <ours/arch/x86/idt.hpp>
#include <ours/arch/x86/descriptor.hpp>

#include <arch/system.hpp>

using arch::Cr4;

namespace ours {
    auto arch_current_cpu() -> CpuNum {  
        return 0; 
    }

    auto arch_activate_cpu(CpuNum) -> Status {  
        return Status::Unimplemented;
    }

    auto arch_deactivate_cpu(CpuNum) -> Status {  
        return Status::Unimplemented; 
    }

    auto x86_init_percpu(CpuNum cpunum) -> void {
        x86_init_feature_percpu();

        // GDT
        x86_load_gdt();

        // IDT
        x86_load_idt();

        Cr4::read().set<Cr4::FsGsBase>(x86_has_feature(CpuFeatureType::FsGsBase))
                   .set<Cr4::Umip>(x86_has_feature(CpuFeatureType::Umip))
                   .write();
    }

} // namespace ours
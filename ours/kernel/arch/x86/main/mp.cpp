#include <ours/arch/cpu.hpp>

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

    auto x86_setup_percpu(CpuNum cpunum) -> void {

    }

} // namespace ours
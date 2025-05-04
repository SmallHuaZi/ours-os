#include <ours/arch/x86/feature.hpp>
#include <ours/cpu-local.hpp>

#include <arch/processor.hpp>

namespace ours {
    CPU_LOCAL
    static arch::X86CpuInfo g_x86_cpu_info;

    auto x86_has_feature(CpuFeatureType type) -> bool {
        auto cpuinfo = CpuLocal::access(&g_x86_cpu_info);
        DEBUG_ASSERT(cpuinfo, "Failed to read CpuInfo");

        return cpuinfo->feature().has_feature(type);
    }

    INIT_CODE
    auto x86_init_feature_percpu() -> void {
        auto cpuinfo = CpuLocal::access(&g_x86_cpu_info);
        cpuinfo->init();
    }

    INIT_CODE
    auto x86_init_feature_early() -> void {
        x86_init_feature_percpu();
    }

} // namespace ours
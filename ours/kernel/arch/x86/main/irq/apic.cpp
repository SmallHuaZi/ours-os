#include <ours/types.hpp>
#include <ours/cpu.hpp>
#include <ustl/collections/static-vec.hpp>

namespace ours {
    static ustl::collections::StaticVec<isize, MAX_CPU>  s_cpu_to_apicid;

    auto cpu_to_apic(CpuNum cpunum) -> isize {
        return s_cpu_to_apicid[cpunum];
    }

} // namespace ours
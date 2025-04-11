#include <ours/types.hpp>
#include <ours/cpu.hpp>
#include <ustl/collections/static-vec.hpp>

namespace ours {
    static ustl::collections::StaticVec<isize, MAX_CPU_NUM>  CPU_TO_APIC;

    auto cpu_to_apic(CpuNum cpunum) -> isize
    {
        return CPU_TO_APIC[cpunum];
    }

} // namespace ours
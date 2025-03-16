#include <ours/types.hpp>
#include <ours/cpu.hpp>
#include <ustl/collections/array.hpp>

namespace ours {
    static ustl::collections::Array<isize, MAX_CPU_NUM>  CPU_TO_APIC;

    auto cpu_to_apic(CpuNum cpunum) -> isize
    {
        return CPU_TO_APIC[cpunum];
    }

} // namespace ours
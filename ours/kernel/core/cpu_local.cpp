#include <ours/cpu_local.hpp>
#include <ours/mem/early.hpp>

#include <arch/cache.hpp>

namespace ours {
    auto CpuLocal::init(CpuId cpuid) -> Status
    {
        if (cpuid._0 == BOOT_CPU_ID._0) {
            return Status::Ok;
        }

        auto const area_size = Self::CPU_LOCAL_END - Self::CPU_LOCAL_START;
        if (auto local_area_start = mem::EarlyMem::allocate<char>(area_size, arch::CACHE_SIZE)) {
            Self::CPU_LOCAL_OFFSET[cpuid._0] = local_area_start - Self::CPU_LOCAL_START;
        }
        return Status::Ok;
    }

} // namespace gktl
#include <ours/cpu_local.hpp>
#include <ours/mem/mod.hpp>

#include <arch/cache.hpp>

namespace ours {
    auto CpuLocal::init(CpuId cpuid) -> Status
    {
        if (cpuid == BOOT_CPU_ID) {
            return Status::Ok;
        }

        auto const nr_frames = Self::area_size() / mem::FRAME_SIZE;

        mem::FrameList<> frame_list;
        if (Status::Ok == mem::alloc_frames(mem::GAF_KERNEL, &frame_list, nr_frames)) {
        }
        return Status::Ok;
    }

} // namespace gktl
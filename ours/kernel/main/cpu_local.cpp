#include <ours/cpu_local.hpp>
#include <ours/mem/mod.hpp>

#include <arch/cache.hpp>

namespace ours {
    extern char STATIC_CPU_LOCAL_START[] LINK_NAME("__cpu_local_start");
    extern char STATIC_CPU_LOCAL_END[] LINK_NAME("__cpu_local_end");

    FORCE_INLINE
    static auto static_cpu_local_area_size() -> usize
    {  return STATIC_CPU_LOCAL_END - STATIC_CPU_LOCAL_START;  }

    ustl::collections::Array<isize, MAX_CPU_NUM>    CpuLocal::UNIT_OFFSET;
    ustl::collections::Array<isize, mem::MAX_NODES> CpuLocal::GROUP_OFFSET;

    struct CpuLocalChunk
    {

    };

    auto CpuLocal::init(CpuId cpuid) -> Status
    {
        if (cpuid == BOOT_CPU_ID) {
            return Status::Ok;
        }

        auto const nr_frames = static_cpu_local_area_size() / mem::FRAME_SIZE;

        mem::FrameList<> frame_list;
        if (Status::Ok == mem::alloc_frames(mem::GAF_KERNEL, &frame_list, nr_frames)) {
        }
        return Status::Ok;
    }

} // namespace gktl
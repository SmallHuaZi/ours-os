#include <ours/cpu_local.hpp>
#include <ours/mem/mod.hpp>

#include <ustl/math/log.hpp>
#include <ustl/mem/align.hpp>

#include <logz4/log.hpp>

using ustl::mem::align_up;

namespace ours {
    auto gktl::CpuLocal::init(CpuId id) -> Status
    {
        if (id == BOOT_CPU_ID) {
            CPU_LOCAL_MAP[BOOT_CPU_ID] = CPU_LOCAL_START;
        } else [[likely]] {
            auto const size_unaligned = CPU_LOCAL_END - CPU_LOCAL_START;
            auto const nr_frames = align_up(size_unaligned, mem::FRAME_SIZE) / mem::FRAME_SIZE;

            mem::FrameList<> frame_list;
            if (Status::Ok != mem::alloc_frames(mem::GAF_KERNEL, &frame_list, nr_frames)) {
                return Status::OutOfMem;
            }

            log::debug("CPU-local variables on CPU {} located [{}, {}]PM", id);
        }

        return Status::Ok;
    }

} // namespace ours
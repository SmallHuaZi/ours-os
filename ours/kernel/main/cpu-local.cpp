#include <ours/cpu-local.hpp>
#include <ours/mem/mod.hpp>
#include <ours/mem/node-mask.hpp>

#include <arch/cache.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/algorithms/generation.hpp>

namespace ours {
    /// Range [kStaticCpuLocalStart, kStaticCpuLocalEnd + kMaxCpuDynamicAreaSizePerCpu) is
    /// the heart of CpuLocal allocator. 

    extern char const kStaticCpuLocalStart[] LINK_NAME("__cpu_local_start");
    extern char const kStaticCpuLocalEnd[] LINK_NAME("__cpu_local_end");

    CXX11_CONSTEXPR
    static auto const kMaxCpuDynamicAreaSizePerCpu = KB(16);

    CXX11_CONSTEXPR
    static auto const kGapBetweenAreas = PAGE_SIZE;

    FORCE_INLINE
    static auto static_cpu_local_area_size() -> usize {
        return kStaticCpuLocalEnd - kStaticCpuLocalStart;
    }

    FORCE_INLINE
    static auto max_cpu_local_area_size() -> usize {
        return MAX_CPU_NUM * (static_cpu_local_area_size() + kMaxCpuDynamicAreaSizePerCpu + kGapBetweenAreas);
    }

    struct CpuLocalChunk {

    };

    static auto init_cpu_local_area_for(CpuNum cpunum) -> Status {
        auto const total_size = static_cpu_local_area_size() + kMaxCpuDynamicAreaSizePerCpu;
        auto const nr_frames = ustl::mem::align_up(total_size, PAGE_SIZE) / PAGE_SIZE;

        auto const nid = mem::cpu_to_node(cpunum);
        auto frame = mem::alloc_frame_n(nid, mem::kGafKernel, nr_frames);
        if (!frame) {
            return Status::OutOfMem;
        }

        return Status::Ok;
    }

    auto CpuLocal::init(CpuMask const &cpus) -> Status {
        mem::NodeMask groups;
        for_each_cpu(cpus, [] (CpuNum cpunum) {
            auto const status = init_cpu_local_area_for(cpunum);
            DEBUG_ASSERT(Status::Ok == status, "");
        });

        init_percpu();
        return Status::Ok;
    }

} // namespace gktl
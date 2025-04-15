#include <ours/cpu-local.hpp>
#include <ours/mem/mod.hpp>
#include <ours/mem/node-mask.hpp>
#include <ours/mem/node-states.hpp>

#include <arch/cache.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/views/span.hpp>
#include <ustl/algorithms/generation.hpp>

namespace ours {
    /// Range [kCpuLocalAreaStart, kStaticCpuLocalEnd + kMaxCpuDynamicAreaSizePerCpu) is
    /// the dummy region for CpuLocal allocator to calculate offset per cpu local object. 
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

    FORCE_INLINE
    static auto calc_offset(void const *cl_base) -> isize {
        extern char const kCpuLocalAreaStart[] LINK_NAME("__cpu_local_start");

        return reinterpret_cast<i8 const *>(cl_base) - 
               reinterpret_cast<i8 const *>(kCpuLocalAreaStart);
    }

    struct DynamicChunk {

    };

    struct DomainInfo {
    };

    struct CpuLocalAreaInfo {
        usize static_size;
        usize dynamic_size;
        ustl::views::Span<DomainInfo> domains;
    };

    static CpuLocalAreaInfo s_cpu_local_area_info;

    auto CpuLocal::init_per_unit(CpuNum cpunum) -> Status {
        auto const nid = mem::cpu_to_node(cpunum);
        auto virt = mem::get_frame(nid, mem::kGafKernel, nr_frames);
        if (!virt) {
            return Status::OutOfMem;
        }
        s_cpu_offset[cpunum] = calc_offset(virt);

        return Status::Ok;
    }

    auto CpuLocal::init_per_domain(NodeId nid) -> Status {
        auto &cpu_unit = mem::node_cpumask(nid);
    }

    auto CpuLocal::init(CpuMask const &cpus) -> Status {
        auto total_size = static_cpu_local_area_size() + kMaxCpuDynamicAreaSizePerCpu;
        total_size = ustl::mem::align_up(total_size, PAGE_SIZE);

        Status status = mem::global_node_states().for_each_possible_if_noerr([&] (NodeId nid) {
            return init_per_domain(nid);
        });

        ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, 0);

        init_percpu();
        return status;
    }

} // namespace gktl
#include <ours/cpu-local.hpp>
#include <ours/mem/mod.hpp>
#include <ours/mem/node-mask.hpp>
#include <ours/mem/node-states.hpp>
#include <ours/mem/early-mem.hpp>

#include <arch/cache.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/views/span.hpp>
#include <ustl/views/inplace_span.hpp>
#include <ustl/algorithms/generation.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/cv.hpp>
#include <ustl/io/binary_reader.hpp>

#include <ktl/page_guard.hpp>

using ustl::mem::align_up;

namespace ours {
    /// Range [kCpuLocalAreaStart, kStaticCpuLocalEnd + kMaxCpuDynamicAreaSizePerCpu) is
    /// the dummy region for CpuLocal allocator to calculate offset per cpu local object. 
    extern char const kStaticCpuLocalStart[] LINK_NAME("__cpu_local_start");
    extern char const kStaticCpuLocalEnd[] LINK_NAME("__cpu_local_end");

    FORCE_INLINE
    static auto static_cpu_local_area_size() -> usize {
        return kStaticCpuLocalEnd - kStaticCpuLocalStart;
    }

    FORCE_INLINE
    static auto early_cpu_local_area_size() -> usize {
        return align_up(static_cpu_local_area_size() + ArchCpuLocal::kDynFirstChunkSize, PAGE_SIZE);
    }

    template <typename T>
    FORCE_INLINE
    static auto calc_offset(T cl_base) -> isize {
        static_assert(sizeof(T) == sizeof(void *), "Missing size");
        return reinterpret_cast<i8 const *>(cl_base) - 
               reinterpret_cast<i8 const *>(kStaticCpuLocalStart);
    }

    struct CpuLocalAreaInfo {
        typedef CpuLocalAreaInfo    Self;

        auto num_units() const -> usize {
            return unit_offset.size();
        }

        auto num_groups() const -> usize {
            return group_offset.size();
        }

        usize unit_size;
        ustl::views::Span<isize> c2u_map;  // CPU -> Unit
        ustl::views::Span<isize> n2g_map;  // Node -> Group 

        ustl::views::Span<isize> unit_offset;
        ustl::views::Span<isize> group_offset;
        ustl::views::Span<u32> group_num_units;
        ustl::views::Span<u32> group_consume; // UNIT(Kib)
        ustl::views::Span<mem::FrameList<>> group_frames;
    };
    static CpuLocalAreaInfo *s_clai;

    static auto calculate_size_of_cpu_local_area_info(usize nr_groups, usize nr_units) -> usize {
        auto size = sizeof(CpuLocalAreaInfo);
        size += MAX_CPU * sizeof(CpuLocalAreaInfo::c2u_map[0]);
        size += MAX_NODE * sizeof(CpuLocalAreaInfo::n2g_map[0]);
        size += nr_units * sizeof(CpuLocalAreaInfo::unit_size[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_offset[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_frames[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_consume[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_num_units[0]);
    }

    static auto calculate_frames_of_cpu_local_area_info(usize nr_groups, usize nr_units) -> usize {
        return align_up(calculate_size_of_cpu_local_area_info(nr_groups, nr_units), PAGE_SIZE) / PAGE_SIZE;
    }

    static auto alloc_cpu_local_area_info(usize nr_groups, usize nr_units) -> CpuLocalAreaInfo * {
        auto const nr_frames = align_up(calculate_size_of_cpu_local_area_info(nr_groups, nr_units), PAGE_SIZE) / PAGE_SIZE;
        auto const order = mem::num_to_order(nr_frames);
        auto const frame = alloc_frame(NodeId(0), mem::kGafBoot, order);
        if (!frame) {
            return nullptr;
        }

        // Let the first frame serve as the mapping data.
        ustl::io::BinaryReader reader(frame_to_virt<u8>(frame), BIT(order));
        CpuLocalAreaInfo *clai = reader.read<CpuLocalAreaInfo>();
        clai->unit_size = 0;
        clai->n2g_map = reader.read_fixed_n<NodeId>(MAX_NODE);
        clai->c2u_map = reader.read_fixed_n<NodeId>(MAX_CPU);
        clai->unit_offset = reader.read_fixed_n<isize>(nr_units);
        clai->group_offset = reader.read_fixed_n<isize>(nr_groups);
        clai->group_frames = reader.read_fixed_n<mem::FrameList<>>(nr_groups);
        clai->group_consume = reader.read_fixed_n<usize>(nr_groups);

        return clai;
    }

    auto CpuLocal::init_early() -> void {
        auto const replica = mem::EarlyMem::allocate(early_cpu_local_area_size(), kPageAlign, 0);
        if (!replica) {
            panic("No enough memory to place boot cpu-local area");
        }

        // The raw cpu local data must be reserved for CpuLocal::init() later.
        ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, replica);
        install(calc_offset(replica));
    }

    auto CpuLocal::init() -> Status {
        auto &nodes = mem::node_possible_mask();
        auto const total_nodes = nodes.size();
        auto const total_units = num_possible_cpus();

        CXX11_CONSTEXPR
        static auto const kMinUnitAlign = ustl::algorithms::max<usize>(ArchCpuLocal::kUnitAlign, PAGE_SIZE);
        auto const dyn_size = ArchCpuLocal::kDynFirstChunkSize;
        auto const size_sum = align_up(static_cpu_local_area_size() + dyn_size, kMinUnitAlign);

        CpuLocalAreaInfo *ai;
        mem::FrameList<> frames;
        ustl::views::Span<CpuNum> u2c_map;
        VirtAddr base = ustl::NumericLimits<VirtAddr>::max();
        auto const total_groups = nodes.count();
        for (auto group = 0, unit = 0, nid = 0; nid < total_nodes; ++nid) {
            if (!nodes.test(nid)) {
                continue;
            }

            // FIXME(SmallHuaZi) If units per group is not aligned to each other. the size bytes of padding
            // 
            auto &cpus = mem::node_cpumask(nid);
            auto const num_units = cpus.count();
            auto const num_frames = align_up(size_sum * num_units, PAGE_SIZE) / PAGE_SIZE;
            auto const order = mem::num_to_order(num_frames);
            auto const padding = ((1 << order) - size_sum) / num_units;

            auto frame = alloc_frame(nid, mem::kGafBoot, order);
            if (!frame) {
                mem::free_frames(&frames);
                return Status::OutOfMem;
            }

            if (!group && !unit) [[unlikely]] {
                // Let the first frame serve as the mapping data.
                ustl::io::BinaryReader reader(frame_to_virt<u8>(frame), BIT(order));
                ai = reader.read<CpuLocalAreaInfo>();
                ai->unit_size = size_sum;
                ai->unit_offset = reader.read_fixed_n<isize>(total_units);
                ai->group_offset = reader.read_fixed_n<isize>(total_groups);
                ai->group_frames = reader.read_fixed_n<mem::FrameList<>>(total_groups);
                ai->group_consume = reader.read_fixed_n<usize>(total_groups);
                ai->n2g_map = reader.read_fixed_n<NodeId>(MAX_NODE);
                ai->c2u_map = reader.read_fixed_n<NodeId>(MAX_CPU);
                u2c_map = reader.read_fixed_n<CpuNum>(total_units);
            }
            ai->n2g_map[nid] = group;
            ai->group_frames[group].push_back(*frame);
            ai->group_consume[group] = BIT(order);
            ai->group_num_units[group] = num_units;

            for (auto cpu = 0; cpu < cpus.size(); ++cpu) {
                if (!cpus.test(cpu)) {
                    continue;
                }

                ai->c2u_map[cpu] = unit;
                u2c_map[unit] = cpu;
                unit += 1;
            }

            group += 1;
            base = ustl::algorithms::min(base, frame_to_virt(frame));
        }

        // Skip first unit.
        for (auto group = 0, unit = 1; group < total_groups; ++group) {
            auto const addr = frame_to_virt(&ai->group_frames[group].front());
            ai->group_offset[group] = addr - base;
            for (auto i = 0; i < ai->group_num_units[group]; ++i, ++unit) {
                auto const this_base = addr + ai->unit_size * i;
                auto const offset = this_base - base;
                ai->unit_offset[unit] = offset;
                s_cpu_offset[u2c_map[unit]] = offset + calc_offset(base);

                ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, reinterpret_cast<u8 *>(this_base));
            }
        }

        // Handle first unit
        auto const addr = frame_to_virt(&ai->group_frames[0].front());
        ai->unit_offset[0] = addr - base;
        s_cpu_offset[u2c_map[0]] = addr - VirtAddr(kStaticCpuLocalStart);

        auto early_base = kStaticCpuLocalStart + CpuLocal::read(s_current_cpu_offset);
        ustl::algorithms::copy_n(early_base ,  early_cpu_local_area_size(), reinterpret_cast<u8 *>(addr));
        mem::free_frame(VirtAddr(early_base), mem::num_to_order(early_cpu_local_area_size()));

        init_percpu();
        return Status::Ok;
    }

} // namespace gktl
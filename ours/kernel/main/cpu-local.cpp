#include <ours/cpu-local.hpp>
#include <ours/mem/mod.hpp>
#include <ours/mem/node-mask.hpp>
#include <ours/mem/node-states.hpp>
#include <ours/mem/early-mem.hpp>

#include <ustl/bit.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/views/span.hpp>
#include <ustl/views/inplace_span.hpp>
#include <ustl/algorithms/generation.hpp>
#include <ustl/traits/ref.hpp>
#include <ustl/traits/cv.hpp>
#include <ustl/io/binary_reader.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <arch/cache.hpp>

#include <ktl/page_guard.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

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

    struct DynChunk: public ustl::collections::intrusive::ListBaseHook<> {
        CXX11_CONSTEXPR
        static usize const kBlockSize = PAGE_SIZE;

        CXX11_CONSTEXPR
        static usize const kBlockBits = ustl::bit_width(kBlockSize) - 1;

        CXX11_CONSTEXPR
        static usize const kMinAllocationGranularity = 4;

        CXX11_CONSTEXPR
        static usize const kMinAllocationGranularityShift = ustl::bit_width(kMinAllocationGranularity) - 1;

        /// Each block has a bitmap to trace the allocation status itself.
        struct AllocationMapPerBlock {
            using Map = ustl::BitSet<kBlockSize / kMinAllocationGranularity>;
            Map inner;
            usize first_free;
            usize free_bytes;
        };

        auto init(VirtAddr base, VirtAddr size) -> void;

        /// Bypass the default `init` interface to help us initialize the first dynamic chunk
        auto init_first_chunk(VirtAddr base, VirtAddr size, ustl::views::Span<AllocationMapPerBlock> allocation_maps) -> void;

        auto find(usize size, AlignVal align) -> ustl::Option<VirtAddr>;

        auto allocate(usize size, AlignVal align) -> VirtAddr;

        auto free(VirtAddr va, usize size) -> void;
    private:
        auto init_inner(VirtAddr base, VirtAddr size) -> void;

        /// Return offset in bitmap.
        auto priv_find(usize num_bits, AlignVal bit_align) -> ustl::Option<VirtAddr>;

        VirtAddr base;
        VirtAddr free_size;
        u16 start_offset;
        u16 end_offset;
        ustl::views::Span<AllocationMapPerBlock>  allocation_maps;
        usize num_frames; // of frames held by the chunk.
    };
    using DynChunkList = ustl::collections::intrusive::List<DynChunk>;

    auto DynChunk::init_inner(VirtAddr base, VirtAddr size) -> void {
        this->base = align_down(base, PAGE_SIZE);
        this->free_size = size;
        this->start_offset = base - this->base;
        this->end_offset = align_up(size, PAGE_SIZE) - size;
    }

    // TODO
    auto DynChunk::init(VirtAddr base, VirtAddr size) -> void {
    }

    FORCE_INLINE
    auto DynChunk::init_first_chunk(VirtAddr base, VirtAddr size, ustl::views::Span<AllocationMapPerBlock> maps) 
        -> void {
        init_inner(base, size);
        allocation_maps = maps;
    }

    auto DynChunk::find(usize size, AlignVal align) -> ustl::Option<VirtAddr> {
        if (align < kMinAllocationGranularity) {
            align = kMinAllocationGranularity;
        }

        size = align_up(size, kMinAllocationGranularity);
        auto const num_bits = size >> kMinAllocationGranularityShift;
        auto const bits_align = align >> kMinAllocationGranularityShift;
        auto maybe_bit_off = priv_find(num_bits, bits_align);
        if (!maybe_bit_off) {
            return ustl::none();
        }

        return ustl::some(base + *maybe_bit_off * kMinAllocationGranularity);
    }

    auto DynChunk::priv_find(usize num_bits, AlignVal bit_align) -> ustl::Option<usize> {
        usize const step_bits = ustl::algorithms::max(bit_align, kMinAllocationGranularityShift);
        // For each all blocks in this chunk. It is a violent algorithm without any optimization.
        for (usize iblock = 0, n = allocation_maps.size(); iblock < n; ++iblock) {
            auto &map = allocation_maps[iblock];
            for (auto ibit = align_up(map.first_free, bit_align); ibit < kBlockBits; ibit += step_bits) {
                if (!map.inner.test_range(ibit, num_bits)) {
                    continue;
                }

                auto const start_bit = iblock * kBlockBits + ibit;
                map.first_free = start_bit + num_bits;
                return ustl::some(start_bit);
            }
        }

        return ustl::none();
    }

    struct CpuLocalAreaInfo {
        static auto global() -> CpuLocalAreaInfo * {
            return s_clai;
        }

        auto init(usize nr_groups, usize nr_units, usize dyn_size) -> void;

        auto add_dyn(VirtAddr base, VirtAddr size) -> Status;

        auto num_units() const -> usize {
            return unit_offset.size();
        }

        auto num_groups() const -> usize {
            return group_offset.size();
        }

        VirtAddr base;
        usize unit_size;
        ustl::views::Span<isize> c2u_map;  // CPU -> Unit
        ustl::views::Span<isize> n2g_map;  // Node -> Group 

        ustl::views::Span<isize> unit_offset;
        ustl::views::Span<isize> group_offset;
        ustl::views::Span<u32> group_num_units;
        ustl::views::Span<u32> group_consume; // UNIT(Kib)
        ustl::views::Span<mem::FrameList<>> group_frames;

        DynChunk first_chunk;   // Start with the cpu local base relocated.
        DynChunkList dyn_chunks;

        static inline CpuLocalAreaInfo *s_clai;
    };

    static auto calculate_init_size_of_clai(usize nr_groups, usize nr_units) -> usize {
        auto size = MAX_CPU * sizeof(CpuLocalAreaInfo::c2u_map[0]);
        size += MAX_NODE * sizeof(CpuLocalAreaInfo::n2g_map[0]);
        size += MAX_NODE * sizeof(CpuLocalAreaInfo::unit_offset[0]);
        size += nr_units * sizeof(CpuLocalAreaInfo::group_offset[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_frames[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_consume[0]);
        size += nr_groups * sizeof(CpuLocalAreaInfo::group_num_units[0]);
        return size;
    }

    static auto calculate_frames_of_clai(usize nr_groups, usize nr_units) -> usize {
        return align_up(calculate_init_size_of_clai(nr_groups, nr_units), PAGE_SIZE) / PAGE_SIZE;
    }

    auto CpuLocalAreaInfo::init(usize nr_groups, usize nr_units, usize dyn_size) -> void {
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

    auto CpuLocal::init(usize dyn_size, usize unit_align) -> Status {
        unit_align = ustl::algorithms::max<usize>(unit_align, PAGE_SIZE);

        auto const unit_size = align_up(static_cpu_local_area_size() + dyn_size, unit_align);

        auto &nodes = mem::node_possible_mask();
        auto const total_groups = nodes.count();
        auto const total_units = num_possible_cpus();
        auto clai = CpuLocalAreaInfo::global();
        // if (Status::Ok != clai->init(total_groups, total_units, dyn_size)) {
            // panic("Failed to intialize glboal cpu local area info descriptor");
        // }

        mem::FrameList<> frames;
        ustl::views::Span<CpuNum> u2c_map;
        VirtAddr base = ustl::NumericLimits<VirtAddr>::max();
        auto const total_nodes = nodes.size();
        for (auto group = 0, unit = 0, nid = 0; nid < total_nodes; ++nid) {
            if (!nodes.test(nid)) {
                continue;
            }

            // FIXME(SmallHuaZi) If units per group is not aligned to each other. the size bytes of padding
            // 
            auto &cpus = mem::node_cpumask(nid);
            auto const num_units = cpus.count();
            auto const num_frames = align_up(unit_size * num_units, PAGE_SIZE) / PAGE_SIZE;
            auto const order = mem::num_to_order(num_frames);
            auto const padding = ((1 << order) - unit_size) / num_units;

            auto frame = alloc_frame(nid, mem::kGafBoot, order);
            if (!frame) {
                mem::free_frames(&frames);
                return Status::OutOfMem;
            }

            clai->n2g_map[nid] = group;
            clai->group_frames[group].push_back(*frame);
            clai->group_consume[group] = BIT(order);
            clai->group_num_units[group] = num_units;

            for (auto cpu = 0; cpu < cpus.size(); ++cpu) {
                if (!cpus.test(cpu)) {
                    continue;
                }

                clai->c2u_map[cpu] = unit;
                u2c_map[unit] = cpu;
                unit += 1;
            }

            group += 1;
            base = ustl::algorithms::min(base, frame_to_virt(frame));
        }
        clai->base = base;
        clai->unit_size = unit_size;

        // Skip first unit.
        for (auto group = 0, unit = 1; group < total_groups; ++group) {
            auto const addr = frame_to_virt(&clai->group_frames[group].front());
            clai->group_offset[group] = addr - base;
            for (auto i = 0; i < clai->group_num_units[group]; ++i, ++unit) {
                auto const this_base = addr + clai->unit_size * i;
                auto const offset = this_base - base;
                clai->unit_offset[unit] = offset;
                s_cpu_offset[u2c_map[unit]] = offset + calc_offset(base);

                ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, reinterpret_cast<u8 *>(this_base));
            }
        }

        // Handle first unit
        auto const addr = frame_to_virt(&clai->group_frames[0].front());
        clai->unit_offset[0] = addr - base;
        s_cpu_offset[u2c_map[0]] = addr - VirtAddr(kStaticCpuLocalStart);

        auto early_base = kStaticCpuLocalStart + CpuLocal::read(s_current_cpu_offset);
        ustl::algorithms::copy_n(early_base ,  early_cpu_local_area_size(), reinterpret_cast<u8 *>(addr));
        mem::free_frame(VirtAddr(early_base), mem::num_to_order(early_cpu_local_area_size()));

        init_percpu();
        return Status::Ok;
    }

    auto CpuLocal::allocate(usize size, AlignVal align, mem::Gaf gaf) -> void * {
        auto clai = CpuLocalAreaInfo::global();
        for (auto chunk : clai->dyn_chunks) {
            auto base = chunk.allocate(size, align);
            if (!base) {
                continue;
            }

            return reinterpret_cast<void *>(base);
        }

        // Here the new dynamic chunk should be created.

        return nullptr;
    }

} // namespace gktl
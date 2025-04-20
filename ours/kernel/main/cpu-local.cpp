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

#include <logz4/log.hpp>
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
        typedef DynChunk    Self;

        CXX11_CONSTEXPR
        static usize const kMinAllocationGranularity = 4;

        CXX11_CONSTEXPR
        static usize const kMinAllocationGranularityShift = ustl::bit_width(kMinAllocationGranularity) - 1;

        CXX11_CONSTEXPR
        static usize const kBlockSize = PAGE_SIZE;

        CXX11_CONSTEXPR
        static usize const kBlockBits = kBlockSize / kMinAllocationGranularity;

        /// Each block has a bitmap to trace the allocation status itself.
        struct AllocationMapPerBlock {
            using Map = ustl::BitSet<kBlockBits>;
            Map inner;
            u32 first_free;
            u32 free_bytes;
            u32 left_free;    // Continual free bytes of block by left side 
            u32 right_free;   // Continual free bytes of block by right side 
            u32 contig_start;
            u32 contig_bits;
            u32 total_bits;
        };

        INIT_CODE
        static auto create_first(VirtAddr base, VirtAddr size) -> DynChunk *;

        auto init(VirtAddr base, VirtAddr size) -> void;

        struct FindResult {
            VirtAddr start;
            usize bit_offset;
            usize num_bits;
        };
        auto find(usize size, AlignVal align) -> ustl::Option<FindResult>;

        auto allocate(usize size, AlignVal align) -> VirtAddr;

        auto free(VirtAddr va, usize size) -> void;

        auto dump() const -> void;
    private:
        /// Common routine for initializing a chunk. Do nothing about memory allocation,
        /// and just to calculte a part of metadata we need.
        ///
        /// Return how many allocation maps this chunk needs.
        auto init_common(VirtAddr base, VirtAddr size) -> usize;

        /// Initialize all allocation maps that has been created.
        auto init_allocation_maps() -> void;

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

    FORCE_INLINE
    auto DynChunk::init_common(VirtAddr base, VirtAddr size) -> usize {
        auto const base_aligned = align_down(base, PAGE_SIZE);
        auto const end_aligned = align_up(base + size, PAGE_SIZE); 
        this->base = base_aligned;
        this->start_offset = base - base_aligned;
        this->end_offset = end_aligned - base - size;
        this->free_size = size;

        return (end_aligned - base_aligned) / kBlockSize;
    }

    FORCE_INLINE
    auto DynChunk::init_allocation_maps() -> void {
        auto const n = allocation_maps.size();
        allocation_maps[0].first_free = start_offset >> kMinAllocationGranularityShift;
        allocation_maps[0].free_bytes = kBlockSize - start_offset;
        allocation_maps[0].left_free = 0;
        allocation_maps[0].right_free = 0;
        allocation_maps[0].contig_start = allocation_maps[0].first_free;
        allocation_maps[0].contig_bits = kBlockBits - allocation_maps[0].first_free;
        allocation_maps[0].total_bits = allocation_maps[0].free_bytes >> kMinAllocationGranularityShift;
        if (n == 1) {
            return;
        }

        for (auto i = 1; i < n; ++i) {
            allocation_maps[i].first_free = 0;
            allocation_maps[i].free_bytes = kBlockSize;
            allocation_maps[i].left_free = kBlockSize;
            allocation_maps[i].right_free = kBlockSize;
            allocation_maps[i].contig_bits = kBlockSize;
            allocation_maps[i].contig_start = 0;
            allocation_maps[i].total_bits = kBlockSize >> kMinAllocationGranularityShift;
        }
        allocation_maps[0].right_free = allocation_maps[1].free_bytes;

        // The last block may has a hole.
        allocation_maps[n - 1].free_bytes -= end_offset >> kMinAllocationGranularityShift;
        allocation_maps[n - 1].right_free = 0;
        allocation_maps[n - 1].contig_bits = allocation_maps[n - 1].free_bytes;
        allocation_maps[n - 1].total_bits = allocation_maps[n - 1].free_bytes >> kMinAllocationGranularityShift;
        if (n == 2) {
            allocation_maps[1].left_free = allocation_maps[0].free_bytes;
        }
    }

    INIT_CODE
    auto DynChunk::create_first(VirtAddr base, VirtAddr size) -> DynChunk * {
        auto self = mem::EarlyMem::allocate<Self>(1, MAX_NODE);
        if (!self) {
            return nullptr;
        }
        ustl::mem::construct_at(self);
        auto nr_blocks = self->init_common(base, size);

        auto maps = mem::EarlyMem::allocate<AllocationMapPerBlock>(nr_blocks, MAX_NODE);
        if (!maps) {
            panic("Failed to allaote first dynamic cpu local chunk");
        }
        ustl::mem::construct_at(maps);
        self->allocation_maps = ustl::views::make_span(maps, nr_blocks);
        self->init_allocation_maps();

        self->dump();
        return self;
    }

    // TODO
    auto DynChunk::init(VirtAddr base, VirtAddr size) -> void {
        // This is the top half
        auto const nr_blocks = init_common(base, size);
        auto raw_maps = new AllocationMapPerBlock[nr_blocks];
        if (!raw_maps) {
            log::error("Failed to allocate allocation maps for dynamic cpu local chunk.");
            return;
        }
        allocation_maps = ustl::views::make_span(raw_maps, nr_blocks);
        init_allocation_maps();
    }

    auto DynChunk::allocate(usize size, AlignVal align) -> VirtAddr {
        auto result = find(size, align);
        if (!result) {
            return 0;
        }
        auto [base, bit_offset, num_bits] = *result; 
        for (auto i = 0; i < num_bits; ++i) {
            allocation_maps[(bit_offset + i) / kBlockBits].inner.set((bit_offset + i) % kBlockBits);
        }
        return base;
    }

    auto DynChunk::find(usize size, AlignVal align) -> ustl::Option<FindResult> {
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

        auto const bit_offset = *maybe_bit_off;
        return FindResult(base + bit_offset * kMinAllocationGranularity, bit_offset, num_bits);
    }

    /// TODO(SmallHuaZi) Find an algorithm to take the place of now.
    auto DynChunk::priv_find(usize alloc_bits, AlignVal bit_align) -> ustl::Option<usize> {
        usize const step_bits = ustl::algorithms::max(bit_align, kMinAllocationGranularityShift);

        usize bits_acc = 0;
        // For each all blocks in this chunk. It is a violent algorithm without any optimization.
        for (usize iblock = 0, n = allocation_maps.size(); iblock < n; ++iblock) {
            auto &map = allocation_maps[iblock];
            for (auto ibit = align_up(map.first_free, bit_align); ibit < map.total_bits; ibit += step_bits) {
                usize bits_test = ustl::algorithms::min(alloc_bits - bits_acc, map.total_bits - ibit);
                if (map.inner.test_range(ibit, bits_test)) {
                    bits_acc = 0;
                    continue;
                }

                if (bits_acc + bits_test < alloc_bits) {
                    bits_acc += bits_test;
                    continue;
                }

                map.first_free = iblock * kBlockBits + ibit + bits_test;
                return ustl::some(map.first_free - alloc_bits);
            }
        }

        return ustl::none();
    }

    auto DynChunk::dump() const -> void {
        log::info("DynChunk: [base: 0x{:X}, so: {}, eo: {}]", base, start_offset, end_offset);
        for (auto &map : allocation_maps) {
            log::info("\t Map: [ff: {}, total: {}]", map.first_free, map.free_bytes);
        }
    }

    struct CpuLocalAreaInfo {
        typedef CpuLocalAreaInfo    Self;

        FORCE_INLINE
        static auto set_global(Self *self) -> void {
            s_clai = self;
        }

        FORCE_INLINE
        static auto global() -> Self * {
            return s_clai;
        }

        auto init(usize nr_groups, usize nr_units, AlignVal unit_align, usize dyn_size) -> Status;

        FORCE_INLINE
        auto add_dyn(DynChunk &dyn_chunk) -> void {
            dyn_chunks.push_back(dyn_chunk);
        }

        auto create_dyn(VirtAddr base, VirtAddr size) -> Status;

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

        DynChunkList dyn_chunks;
        static inline CpuLocalAreaInfo *s_clai;
    };

    auto CpuLocalAreaInfo::init(usize nr_groups, usize nr_units, AlignVal unit_align, usize dyn_size) -> Status {
        usize num_allocated = 0;
        ustl::Array<usize *, 8> allocated;

        auto const do_reclaim = [&] () {
            for (auto i = 0; i < num_allocated; ++i) {
                delete[] allocated[i];
            }
        };
    
#define MEMBER_INIT_IF_ERROR_THEN_RETURN(Member, Number) \
        auto raw_##Member = new ustl::traits::RemoveCvRefT<decltype(CpuLocalAreaInfo::Member[0])>[Number];\
        if (!raw_##Member) {\
            log::trace("Failed to allocate map for "#Member);\
            do_reclaim();\
            return Status::OutOfMem;\
        }\
        num_allocated += 1;\
        Member = ustl::views::make_span(raw_##Member, Number);

        MEMBER_INIT_IF_ERROR_THEN_RETURN(n2g_map, MAX_NODE);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(c2u_map, MAX_CPU);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(unit_offset, nr_units);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_offset, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_num_units, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_consume, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_frames, nr_groups);
#undef MEMBER_INIT_IF_ERROR_THEN_RETURN

        unit_size = ustl::mem::align_up(static_cpu_local_area_size() + dyn_size, unit_align);
        return Status::Ok;
    }

    auto CpuLocal::init_early() -> void {
        auto clai = mem::EarlyMem::allocate<CpuLocalAreaInfo>(1, MAX_NODE);
        if (!clai) {
            panic("Failed to allocate cpu local area info");
        }
        ustl::mem::construct_at(clai);

        auto size_sum = early_cpu_local_area_size();
        auto const dyn_size = size_sum - static_cpu_local_area_size();
        auto const replica = mem::EarlyMem::allocate<u8>(early_cpu_local_area_size(), kPageAlign, NodeId(MAX_NODE));
        if (!replica) {
            panic("No enough memory to place boot cpu-local area");
        }
        log::trace("Early cpu-local area start at 0x{:X}", VirtAddr(replica));

        auto const dyn_base = VirtAddr(replica) + static_cpu_local_area_size();
        auto first_chunk = DynChunk::create_first(dyn_base, dyn_size);
        if (!first_chunk) {
            panic("Failed to allaote first dynamic cpu local chunk");
        }
        log::trace("Early first chunk start at 0x{:X}", dyn_base);
        clai->add_dyn(*first_chunk);
        CpuLocalAreaInfo::set_global(clai);

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
        if (!clai) {
            panic("Failed to allocat CLAI");
        }
        clai->init(total_groups, total_units, unit_align, dyn_size);

        mem::FrameList<> frames;
        ustl::views::Span<CpuNum> u2c_map = ustl::views::make_span(new CpuNum[total_units], total_units);
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

        delete[] u2c_map.data();
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

        // Here the new dynamic chunk should be create_globald.

        return nullptr;
    }

    auto CpuLocal::do_free(void *ptr) -> void {

    }

} // namespace gktl
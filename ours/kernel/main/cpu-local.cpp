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

#include <ktl/new.hpp>
#include <logz4/log.hpp>

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
        auto raw_maps = new (mem::kGafKernel) AllocationMapPerBlock[nr_blocks];
        if (!raw_maps) {
            log::error("Failed to allocate allocation maps for dynamic cpu local chunk.");
            return;
        }
        allocation_maps = ustl::views::make_span(raw_maps, nr_blocks);
        init_allocation_maps();
    }

    auto DynChunk::allocate(usize size, AlignVal align) -> VirtAddr {
        log::info("CPU-local allocation action: {}, {}", size, align);
        auto result = find(size, align);
        if (!result) {
            return 0;
        }
        auto [base, bit_offset, num_bits] = *result; 
        for (auto i = 0; i < num_bits; ++i) {
            allocation_maps[(bit_offset + i) / kBlockBits].inner.set((bit_offset + i) % kBlockBits);
        }

        // dump();
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

                map.first_free = ibit + bits_test;
                return ustl::some(iblock * kBlockBits + ibit + bits_test - alloc_bits);
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

        auto build_map_info() -> Status;

        auto build_area_info() -> Status;

        auto dump() const -> void;

        FORCE_INLINE
        auto add_dyn(DynChunk &dyn_chunk) -> void {
            dyn_chunks_.push_back(dyn_chunk);
        }

        auto create_dyn(VirtAddr base, VirtAddr size) -> Status;

        FORCE_INLINE CXX11_CONSTEXPR
        auto cpu_to_unit(CpuNum cpu) -> usize {
            return c2u_map_[cpu];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto unit_to_cpu(usize unit) -> CpuNum {
            return u2c_map_[unit];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto node_to_group(NodeId nid) -> usize {
            return n2g_map_[nid];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto group_to_node(usize group) -> NodeId {
            return g2n_map_[group];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto num_units_on_group(usize group) const -> usize {
            return group_num_units_[group];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto num_units() const -> usize {
            return unit_offset_.size();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto num_groups() const -> usize {
            return group_offset_.size();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto group_base(usize group) const -> VirtAddr {
            return base_ + group_offset_[group];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto unit_base(usize unit) const -> VirtAddr {
            return base_ + unit_offset_[unit];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto cpu_offset(CpuNum cpu) const -> VirtAddr {
            return unit_offset_[c2u_map_[cpu]];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto cpu_base(CpuNum cpu) const -> VirtAddr {
            return base_ + unit_offset_[c2u_map_[cpu]];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto node_base(NodeId nid) const -> VirtAddr {
            return base_ + group_offset_[n2g_map_[nid]];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto node_offset(NodeId nid) const -> VirtAddr {
            return group_offset_[n2g_map_[nid]];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto unit_size() const -> VirtAddr {
            return unit_size_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto unit_offset(usize unit) const -> VirtAddr {
            return unit_offset_[unit];
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto base() const -> VirtAddr {
            return base_;
        }

        template <typename T>
        FORCE_INLINE
        auto fix_addr(T addr) -> VirtAddr {
            return reinterpret_cast<VirtAddr>(addr) - (calc_offset(base_));
        }

        template <typename T, typename U>
        FORCE_INLINE
        auto fix_addr(U addr) -> T * {
            return reinterpret_cast<T *>(fix_addr(addr));
        }

        VirtAddr base_;
        usize unit_size_;
        ustl::views::Span<isize> c2u_map_;  // CPU -> Unit
        ustl::views::Span<isize> u2c_map_;  // Unit -> CPU 
        ustl::views::Span<isize> n2g_map_;  // Node -> Group
        ustl::views::Span<isize> g2n_map_;  // Group -> Node

        ustl::views::Span<isize> unit_offset_;
        ustl::views::Span<isize> group_offset_;
        ustl::views::Span<u32> group_num_units_;
        ustl::views::Span<u32> group_consume_; // UNIT(Kib)
        ustl::views::Span<mem::FrameList<>> group_frames_;

        DynChunkList dyn_chunks_;
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
            auto raw_##Member = new (mem::kGafKernel) ustl::traits::RemoveCvRefT<decltype(CpuLocalAreaInfo::Member[0])>[Number];\
            if (!raw_##Member) {\
                log::trace("Failed to allocate map for "#Member);\
                do_reclaim();\
                return Status::OutOfMem;\
            }\
            num_allocated += 1;\
            Member = ustl::views::make_span(raw_##Member, Number);

        MEMBER_INIT_IF_ERROR_THEN_RETURN(n2g_map_, MAX_NODE);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(g2n_map_, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(c2u_map_, MAX_CPU);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(u2c_map_, nr_units);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(unit_offset_, nr_units);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_offset_, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_num_units_, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_consume_, nr_groups);
        MEMBER_INIT_IF_ERROR_THEN_RETURN(group_frames_, nr_groups);
        #undef MEMBER_INIT_IF_ERROR_THEN_RETURN

        unit_size_ = ustl::mem::align_up(static_cpu_local_area_size() + dyn_size, unit_align);
        return Status::Ok;
    }

    auto CpuLocalAreaInfo::build_map_info() -> Status {
        auto &nodes = mem::node_possible_mask();
        for (auto group = 0, unit = 0, nid = 0; nid < nodes.size(); ++nid) {
            if (!nodes.test(nid)) {
                continue;
            }

            // FIXME(SmallHuaZi) If units per group is not aligned to each other. the size bytes of padding
            // 
            auto &cpus = mem::node_cpumask(nid);
            n2g_map_[nid] = group;   // nid->group
            g2n_map_[group] = nid;   // group->nid
            group_num_units_[group] = cpus.count();

            for (auto cpu = 0; cpu < cpus.size(); ++cpu) {
                if (!cpus.test(cpu)) {
                    continue;
                }

                c2u_map_[cpu] = unit;
                u2c_map_[unit] = cpu;
                unit += 1;
            }

            group += 1;
        }

        return Status::Ok;
    }

    auto CpuLocalAreaInfo::build_area_info() -> Status {
        auto const nr_groups = num_groups();

        mem::FrameList<> frame_tracker;
        base_ = ustl::NumericLimits<VirtAddr>::max();
        for (auto group = 0; group < nr_groups; ++group) {
            auto const nid = g2n_map_[group];
            auto const nr_units = group_num_units_[group];

            auto const num_frames = align_up(unit_size_ * nr_units, PAGE_SIZE) / PAGE_SIZE;
            auto const order = mem::num_to_order(num_frames);
            auto const padding = (PAGE_SIZE * (1 << order) - unit_size_) / nr_units;

            auto frame = alloc_frame(nid, mem::kGafBoot, order);
            if (!frame) {
                mem::free_frames(&frame_tracker);
                return Status::OutOfMem;
            }

            group_frames_[group].push_back(*frame);
            group_consume_[group] = BIT(order);

            base_ = ustl::algorithms::min(base_, mem::frame_to_virt(frame));
        }

        for (auto group = 0, unit = 0; group < nr_groups; ++group) {
            auto const addr = frame_to_virt(&group_frames_[group].front());
            group_offset_[group] = addr - base_;

            for (auto i = 0; i < group_num_units_[group]; ++i, ++unit) {
                unit_offset_[unit] = addr + unit_size_ * i - base_;
            }
        }

        return Status::Ok;
    }

    auto CpuLocalAreaInfo::dump() const -> void {
        log::info("CpuLocal internal area information: ");
        log::info("  Base of static area : {:X}", VirtAddr(kStaticCpuLocalStart));
        log::info("  Size of static area: {:X}", static_cpu_local_area_size());
        log::info("  Base address: {:X}", base_);
        log::info("  Unit size: {:X}", unit_size_);
        log::info("  Information per CPU : ");
        for (auto i = 0; i < MAX_CPU; ++i) {
            if (cpu_possible_mask().test(i)) {
                log::info("    CPU[{}]: At 0x{:X}, offset={:X}", i, cpu_base(i), cpu_offset(i));
            }
        }
        log::info("  Information per NODE:");
        for (auto i = 0; i < MAX_NODE; ++i) {
            if (mem::node_possible_mask().test(i)) {
                log::info("    NODE[{}]: At 0x{:X}, offset={:X}", i, node_base(i), node_offset(i));
            }
        }
        log::info("Information for first frame in group:");
        for (auto i = 0; i < num_groups(); ++i) {
            auto &first_frame = group_frames_[i].front();
            log::info("  NODE[{}]: At 0x{:0X}, order={}", i, frame_to_virt(&first_frame), first_frame.order());
        }
    }

    auto CpuLocal::init_early() -> void {
        auto clai = mem::EarlyMem::allocate<CpuLocalAreaInfo>(1, MAX_NODE);
        if (!clai) {
            panic("Failed to allocate cpu local area info");
        }
        ustl::mem::construct_at(clai);

        auto size_sum = early_cpu_local_area_size();
        auto const dyn_size = size_sum - static_cpu_local_area_size();
        auto const replica = mem::EarlyMem::allocate<u8>(early_cpu_local_area_size(), PAGE_SIZE, NodeId(MAX_NODE));
        if (!replica) {
            panic("No enough memory to place boot cpu-local area");
        }
        log::trace("Early cpu-local area start at 0x{:X}", VirtAddr(replica));

        auto dyn_base = replica + static_cpu_local_area_size();
        ustl::algorithms::fill_n(dyn_base, dyn_size, 0);

        auto first_chunk = DynChunk::create_first(VirtAddr(dyn_base), dyn_size);
        if (!first_chunk) {
            panic("Failed to allaote first dynamic cpu local chunk");
        }
        log::trace("Early first chunk start at 0x{:X}", VirtAddr(dyn_base));
        clai->base_ = VirtAddr(replica);
        clai->add_dyn(*first_chunk);
        CpuLocalAreaInfo::set_global(clai);

        // The raw cpu local data must be reserved for CpuLocal::init() later.
        ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, replica);

        // First installation is forced to use ArchCpuLocal::install. That is out of the CpuLocal::install
        // being going to read CpuLocal::cpunum()
        ArchCpuLocal::install(usize(calc_offset(replica)));

        ArchCpuLocal::write(reinterpret_cast<usize>(&s_current_cpu_offset), usize(calc_offset(replica)));

        // The Bootstrap CPU always has the zero number.
        ArchCpuLocal::write(reinterpret_cast<usize>(&s_current_cpu), 0);

        // Mark installed.
        s_installed.set(0, true);
    }

    auto CpuLocal::init(usize dyn_size, usize unit_align) -> Status {
        auto &nodes = mem::node_possible_mask();
        auto clai = CpuLocalAreaInfo::global();

        auto status = clai->init(nodes.count(), num_possible_cpus(), unit_align, dyn_size) ;
        if (Status::Ok != status) {
            log::error("Failed to initialize cpu local internal area info with given reason: {}", to_string(status));
            return status; 
        }

        status = clai->build_map_info();
        if (Status::Ok != status) {
            log::error("Failed to initialize map tables with given reason: {}", to_string(status));
            return status; 
        }

        status = clai->build_area_info();
        if (Status::Ok != status) {
            log::error("Failed to initialize area infos with given reason: {}", to_string(status));
            return status; 
        }

        auto const delta = calc_offset(clai->base());
        for (usize i = 0, n = clai->num_units(); i < n; ++i) {
            auto cpu = clai->unit_to_cpu(i);
            s_cpu_offset[cpu] = clai->unit_offset(i) + delta;

            auto const unit_base = reinterpret_cast<u8 *>(clai->unit_base(i));
            if (cpu == 0) {
                auto base = kStaticCpuLocalStart + CpuLocal::read(s_current_cpu_offset);
                ustl::algorithms::copy_n(base, early_cpu_local_area_size(), unit_base);
                mem::free_frame(VirtAddr(base), mem::num_to_order(early_cpu_local_area_size()));
            } else [[likely]] {
                ustl::algorithms::copy(kStaticCpuLocalStart, kStaticCpuLocalEnd, unit_base);
            }
        }

        s_installed.set(0, false);
        // Bootstrap CPU always has the zero number.
        init_percpu(0);

        {
            clai->dump();

            // Range check
            DEBUG_ASSERT(usize(kStaticCpuLocalStart) <= usize(&s_current_cpu)); 
            DEBUG_ASSERT(usize(&s_current_cpu) <= usize(kStaticCpuLocalEnd));

            log::trace("CPU-Local({:X}, {:X})", usize(kStaticCpuLocalStart), usize(kStaticCpuLocalEnd));
            log::trace("Current CPU {:X}", usize(&s_current_cpu));

            auto expected = Self::access(&s_current_cpu);

            {
                auto base = arch::MsrIo::read<usize>(arch::MsrRegAddr::IA32GsBase);
                DEBUG_ASSERT(base + usize(&s_current_cpu) == usize(expected));
            }

            {
                auto const diff = usize(&s_current_cpu) - usize(kStaticCpuLocalStart);
                auto const clbase = CpuLocalAreaInfo::global()->unit_base(CpuLocalAreaInfo::global()->cpu_to_unit(0));            
                DEBUG_ASSERT(clbase + diff == usize(expected));
            }

            {
                auto default_get = CpuLocal::access(&s_current_cpu);
                auto special_get = CpuLocal::access(&s_current_cpu, 0);
                DEBUG_ASSERT(default_get == special_get, "Should equal");
            }
        }

        return Status::Ok;
    }

    auto CpuLocal::allocate(usize size, AlignVal align, mem::Gaf gaf) -> void * {
        auto clai = CpuLocalAreaInfo::global();
        for (auto chunk : clai->dyn_chunks_) {
            auto base = chunk.allocate(size, align);
            if (!base) {
                continue;
            }

            return clai->fix_addr<void>(base);
            // return reinterpret_cast<void *>(base);
        }

        // Here the new dynamic chunk should be create_globald.

        return nullptr;
    }

    auto CpuLocal::do_free(void *ptr) -> void {
        panic("Not support");
    }

    auto CpuLocal::check_addr(VirtAddr va, CpuNum cpu) -> bool {
        auto clai = CpuLocalAreaInfo::global();
        auto start = clai->unit_base(clai->cpu_to_unit(cpu));
        return va >= start && va < start + clai->unit_size();
    }

    auto CpuLocal::dump() -> void {
        DEBUG_ASSERT(CpuLocalAreaInfo::global());
        CpuLocalAreaInfo::global()->dump();

        auto &cpu = cpu_online_mask();
        log::info("CpuLocal public offset information: ");
        for (auto i = 0 ; i < std::size(s_cpu_offset); ++i) {
            if (cpu.test(i)) {
                log::info("  CPU[{}]={}", i, s_cpu_offset[i]);
            }
        }
    }

} // namespace gktl
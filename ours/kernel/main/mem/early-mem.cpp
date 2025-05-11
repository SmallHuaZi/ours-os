#include <ours/mem/early-mem.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/mem/memory_model.hpp>
#include <ours/mem/vmm.hpp>
#include <ours/phys/handoff.hpp>

#include <logz4/log.hpp>
#include <ustl/mem/align.hpp>

namespace ours::mem {
    INIT_CODE
    auto EarlyMem::dump() -> void {
        // Dump all retions in bootmem, should remove in near future.
        mem::EarlyMem::IterationContext context{bootmem::RegionType::Normal};
        log::trace("Memory information from BootMem");
        log::trace("|Domain | {:18} | {:18}", "Base", "Size");
        while (auto region = mem::EarlyMem::iterate(context)) {
            log::trace("|{:6x} | 0x{:16X} | 0x{:16X}", region->nid(), region->base, region->size);
        }
    }

    INIT_CODE
    auto EarlyMem::get_pfn_range_node(NodeId nid) -> gktl::Range<Pfn> {
        Pfn start = ustl::NumericLimits<Pfn>::max(), end = 0;
        EarlyMem::IterationContext context(bootmem::RegionType::Normal, nid);
        while (auto region = EarlyMem::iterate(context)) {
            start = ustl::algorithms::min(start, phys_to_pfn(region->base));
            end = ustl::algorithms::max(end, phys_to_pfn(region->end()));
        }

        return { start, end };
    }

    INIT_CODE
    auto EarlyMem::count_present_frames(Pfn start, Pfn end) -> usize {
        usize nr_presents = 0;
        EarlyMem::IterationContext context(start, end, bootmem::RegionType::Normal, MAX_NODE);
        while (auto region = iterate(context)) {
            if (region->size < PAGE_SHIFT) {
                continue;
            }
            auto base = ustl::mem::align_up(region->base, PAGE_SHIFT);
            auto end = ustl::mem::align_down(region->end(), PAGE_SHIFT);
            if (end - base < PAGE_SHIFT) {
                continue;
            }

            nr_presents += (end - base) / PAGE_SHIFT;
        }

        return nr_presents;
    }

    INIT_CODE
    static auto free_frames_pfn_range(Pfn start, Pfn end) -> void {
        while (start < end) {
            usize order = MAX_FRAME_ORDER;
            while (start + BIT(order) > end) {
                order -= 1;
            }

            free_frame(pfn_to_frame(start), order);
            start += BIT(order);
        }
    }

    INIT_CODE FORCE_INLINE 
    static auto free_frames_phys_range(PhysAddr start, PhysAddr end) -> void {
        auto const start_pfn = phys_to_pfn(start);
        auto const end_pfn = ustl::algorithms::min(phys_to_pfn(end), max_pfn());
        if (start_pfn < end_pfn) {
            free_frames_pfn_range(start_pfn, end_pfn);
        }
    }

    INIT_CODE
    static auto free_unused_memory() -> void {
        EarlyMem::IterationContext context(bootmem::RegionType::Unused, MAX_NODE);
        while (auto region = EarlyMem::iterate(context)) {
            free_frames_phys_range(region->base, region->end());
        }
    }

    INIT_CODE
    auto EarlyMem::do_handoff() -> void {
        free_unused_memory();
    }

    INIT_CODE
    auto EarlyMem::reclaim_init_area() -> void {
        auto start = PhysMap::virt_to_phys(kKernelInitStart);
        auto end = PhysMap::virt_to_phys(kKernelInitEnd);

        if (end < start || end - start < PAGE_SIZE) {
            return;
        }

        free_frames_phys_range(start, end);
    }

} // namespace ours::mem
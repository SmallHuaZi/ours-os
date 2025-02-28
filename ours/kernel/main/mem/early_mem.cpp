#include <ours/mem/early_mem.hpp>
#include <ours/phys/handoff.hpp>

#include <ustl/mem/align.hpp>

namespace ours::mem {
    INIT_DATA
    bootmem::IBootMem *EarlyMem::BOOTMEM;

    auto EarlyMem::init(phys::MemoryHandoff &handoff) -> void
    {}

    auto EarlyMem::get_node_pfn_range(NodeId nid) -> gktl::Range<Pfn>
    {
        Pfn start = ustl::NumericLimits<Pfn>::max(), end = 0;
        bootmem::IterationContext context(nid, bootmem::RegionType::AllType);
        while (auto region = EarlyMem::iterate(context)) {
            start = ustl::algorithms::min(start, phys_to_pfn(region->base));
            end = ustl::algorithms::max(end, phys_to_pfn(region->end()));
        }

        return { start, end };
    }

    auto EarlyMem::count_present_frames(Pfn start, Pfn end) -> usize
    {
        usize nr_presents = 0;
        bootmem::IterationContext context(MAX_NODES, bootmem::RegionType::AllType, start, end);
        while (auto region = BOOTMEM->iterate(context)) {
            if (region->size < FRAME_SIZE) {
                continue;
            }
            auto base = ustl::mem::align_up(region->base, FRAME_SIZE);
            auto end = ustl::mem::align_down(region->end(), FRAME_SIZE);
            if (end - base < FRAME_SIZE) {
                continue;
            }

            nr_presents += (end - base) / FRAME_SIZE;
        }

        return nr_presents;
    }

} // namespace ours::mem
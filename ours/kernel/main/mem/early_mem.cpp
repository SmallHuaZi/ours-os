#include <ours/mem/early_mem.hpp>
#include <ours/phys/handoff.hpp>

#include <ustl/mem/align.hpp>

namespace ours::mem {
    auto EarlyMem::init(phys::MemoryHandoff &handoff) -> void
    {}

    auto EarlyMem::get_node_pfn_range(NodeId nid) -> gktl::Range<Pfn>
    {
        Pfn start = ustl::NumericLimits<Pfn>::max(), end = 0;
        EarlyMem::IterationContext context(bootmem::RegionType::Normal, nid);
        while (auto region = EarlyMem::iterate(context)) {
            start = ustl::algorithms::min(start, phys_to_pfn(region->base));
            end = ustl::algorithms::max(end, phys_to_pfn(region->end()));
        }

        return { start, end };
    }

    auto EarlyMem::count_present_frames(Pfn start, Pfn end) -> usize
    {
        usize nr_presents = 0;
        EarlyMem::IterationContext context(start, end, bootmem::RegionType::Normal, MAX_NODES);
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

} // namespace ours::mem
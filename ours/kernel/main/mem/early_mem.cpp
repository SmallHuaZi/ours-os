#include "ustl/algorithms/minmax.hpp"
#include <ours/mem/early_mem.hpp>
#include <ours/phys/handoff.hpp>

namespace ours::mem {
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

} // namespace ours::mem
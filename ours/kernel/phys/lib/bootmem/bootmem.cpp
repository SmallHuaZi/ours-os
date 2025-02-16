#include <bootmem/bootmem.hpp>

#include <ustl/limits.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/algorithms/minmax.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace bootmem {
    auto IBootMem::get_range_for_nid(NodeId nid) const -> gktl::Range<PhysAddr>
    {
        PhysAddr start = ustl::NumericLimits<PhysAddr>::max(), end = 0;
        IterationContext context(nid, RegionType::AllType, start_address_, end_address_);
        while (auto region = this->iterate(context)) {
            start = ustl::algorithms::min(start, region->base);
            end = ustl::algorithms::max(end, region->end());
        }

        return { start, end };
    }

    auto IBootMem::count_present_blocks(PhysAddr start, PhysAddr end, usize bsize, usize balign, NodeId nid) const -> usize
    {
        usize nr_presents = 0;
        IterationContext context(nid, RegionType::AllType, start, end);
        while (auto region = this->iterate(context)) {
            if (region->size < bsize) {
                continue;
            }
            auto base = align_up(region->base, balign);
            auto end = align_down(region->end(), balign);
            if (end - base < bsize) {
                continue;
            }

            nr_presents += (end - base) / bsize;
        }

        return nr_presents;
    }

} // namespace bootmem
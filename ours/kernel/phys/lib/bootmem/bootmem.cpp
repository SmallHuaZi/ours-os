#include <bootmem/bootmem.hpp>

#include <ustl/limits.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/algorithms/minmax.hpp>

using ustl::mem::align_up;
using ustl::mem::align_down;

namespace bootmem {
    auto IBootMem::count_present_blocks(PhysAddr start, PhysAddr end, usize bsize, usize balign, NodeId nid) const -> usize
    {
        usize nr_presents = 0;
        IterationContext context(this, nid, RegionType::Normal, start, end);
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
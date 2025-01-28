#include <ours/mem/details/sparse_model.hpp>

namespace ours::mem {
    auto SparseMemoryModel::init(ktl::Span<MemRegion> &ranges) -> Status
    {
        return Status::Ok;
    }

    auto SparseMemoryModel::phys_to_frame(PhysAddr) -> PmFrame * 
    {
    }

    auto SparseMemoryModel::populate_range(Pfn start_pfn, Pfn end_pfn) -> Status
    {
        return Status::Unimplemented;
    }

    auto SparseMemoryModel::depopulate_range(Pfn start_pfn, Pfn end_pfn) -> Status
    {
        return Status::Unimplemented;
    }

} // namespace ours::mem
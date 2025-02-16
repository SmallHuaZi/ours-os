#include <ours/mem/details/sparse_model.hpp>

namespace ours::mem {
    auto SparseMemoryModel::init() -> Status
    {
        return Status::Ok;
    }

    auto SparseMemoryModel::phys_to_frame(PhysAddr phys_addr) -> PmFrame * 
    {
        Pfn pfn = phys_to_pfn(phys_addr);
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
#include <ours/mem/memory_model.hpp>

namespace ours::mem {
    auto MemoryModel::init() -> Status
    {
        return Status::Ok;
    }

    auto MemoryModel::populate(Pfn start, usize nr_frames, NodeId nid, Altmap *altmap) -> Status
    {
        return Status::Unimplemented;
    }

    auto MemoryModel::depopulate(Pfn start_pfn, Pfn end_pfn) -> Status
    {
        return Status::Unimplemented;
    }

} // namespace ours::mem
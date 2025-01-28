#include <ours/arch/vm_aspace.hpp>

namespace ours::mem {
    ArchVmAspace::ArchVmAspace()
    {}

    auto ArchVmAspace::init(gktl::Range<VirtAddr>, u64 flags) -> Status
    {}
}
#include <ours/mem/stack.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>

namespace ours::mem {
    NO_MANGLE VirtAddr g_kernel_stack_bottom = 0;
    NO_MANGLE usize    g_kernel_stack_size = 0;

    auto Stack::create(usize size, Self *out) -> Status {
    }

} // namespace ours::mem
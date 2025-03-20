#include <ours/mem/stack.hpp>

namespace ours::mem {
    NO_MANGLE VirtAddr g_kernel_stack_bottom = 0;
    NO_MANGLE usize    g_kernel_stack_size = 0;

} // namespace ours::mem
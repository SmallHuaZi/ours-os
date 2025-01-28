#include <ours/kernel_layout.hpp>

#include <ours/mem/constant.hpp>

namespace ours::mem {
    static auto reserve_kernel_image() -> void
    {
        auto const kernel_start = reinterpret_cast<usize>(KERNEL_CORE_START);
        auto const kernel_end = reinterpret_cast<usize>(KERNEL_CORE_END);

        // Reserve zero page and the kernel image.
        // G_EARLY_ALLOC.protect({ 0, PAGE_SIZE, MemRegion::NoMap });
        // G_EARLY_ALLOC.protect({ kernel_start, kernel_end, MemRegion::Mirror });
    }

    extern "C" auto init_arch() -> void
    {
        reserve_kernel_image();

    }
}
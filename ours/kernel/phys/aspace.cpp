#include <ours/phys/aspace.hpp>

namespace ours::phys {
    struct PageAllocator {
        FORCE_INLINE
        static auto alloc_page(usize n) -> PhysAddr {
            DEBUG_ASSERT(global_bootmem());
            return global_bootmem()->allocate(PAGE_SIZE * n, PAGE_SIZE);
        }

        FORCE_INLINE
        static auto free_page(void *page, usize n) -> void {
            DEBUG_ASSERT(global_bootmem());
            global_bootmem()->deallocate(PhysAddr(page), PAGE_SIZE * n);
        }
    };

} // namespace ours::phys
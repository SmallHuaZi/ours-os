#include <ours/phys/init.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/handoff.hpp>

namespace ours::phys {
    Aspace   *ASPACE = 0;
    Handoff  *HANDOFF = 0;
    bootmem::IBootMem  *BOOTMEM = 0;

    auto setup_init_data() -> void {
        DEBUG_ASSERT(global_bootmem(), "Never be null at here");
        auto addr = global_bootmem()->allocate(sizeof(Handoff), PAGE_SIZE);
        if (!addr) {
            panic("No memory in {}", __func__);
        }
        HANDOFF = reinterpret_cast<decltype(HANDOFF)>(addr);

        addr = global_bootmem()->allocate(sizeof(*ASPACE), alignof(Aspace));
        if (!addr) {
            panic("No memory in {}", __func__);
        }
        ASPACE = reinterpret_cast<decltype(ASPACE)>(addr);
    }

    auto probe_topology() -> void {

    }

} // namespace ours::phys
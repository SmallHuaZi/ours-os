#include <ours/phys/init.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/handoff.hpp>

namespace ours::phys {
    Handoff  *gHandoff = 0;
    bootmem::IBootMem  *gBootMem = 0;

    auto setup_init_data() -> void {
        DEBUG_ASSERT(global_bootmem(), "Never be null at here");
        auto addr = global_bootmem()->allocate(sizeof(Handoff), PAGE_SIZE);
        if (!addr) {
            panic("No memory in {}", __func__);
        }
        gHandoff = reinterpret_cast<decltype(gHandoff)>(addr);
    }

    auto probe_topology() -> void {

    }

} // namespace ours::phys
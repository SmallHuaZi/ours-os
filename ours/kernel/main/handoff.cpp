#include <ours/phys/handoff.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/early-mem.hpp>
#include <ours/mem/init.hpp>
#include <ours/mem/vmm.hpp>
#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/panic.hpp>

#include <logz4/log.hpp>

namespace ours {
    using mem::PhysMap;
    using phys::Handoff;
    using phys::MemoryHandoff;

    INIT_DATA
    phys::Handoff *g_phys_handoff = 0;

    INIT_CODE
    auto setup_handoff(PhysAddr phys_addr) -> void {
        log::trace("Hand off BootMem to EarlyMem");

        auto const handoff = PhysMap::phys_to_virt<Handoff>(phys_addr);
        DEBUG_ASSERT(handoff->verify(), "Pass a error handoff");

        g_phys_handoff = handoff;
        mem::handoff_early_pmm(g_phys_handoff->mem);

    }

} // namespace ours
#include <ours/phys/handoff.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/init.hpp>
#include <ours/start.hpp>
#include <ours/panic.hpp>

namespace ours {
    using mem::PhysMap;
    using phys::Handoff;
    using phys::MemoryHandoff;

    INIT_DATA
    phys::Handoff *PHYS_HANDOFF = 0;

    INIT_CODE
    auto setup_handoff(PhysAddr phys_addr) -> void
    {
        auto const handoff = PhysMap::phys_to_virt<Handoff>(phys_addr);
        handoff->verify();

        PHYS_HANDOFF = handoff;
    }

} // namespace ours
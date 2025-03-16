#include <ours/panic.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ours/phys/console.hpp>

#include "../legacy-boot.hpp"

#include <gktl/static_objects.hpp>
#include <ustl/lazy_init.hpp>
#include <arch/system.hpp>

namespace ours::phys {
    NO_MANGLE NO_RETURN
    auto phys_main(PhysAddr loader_param) -> void
    {
        gktl::init_static_objects();
        init_early_console();
        println("loader params at 0x{:X}", loader_param);

        init_memory(loader_param, global_aspace());

        /// Here the bootmem is available. We first allocate the init data by it.
        setup_init_data();

        probe_topology();

        panic("Never reach at here");
    }

} // ours::phys
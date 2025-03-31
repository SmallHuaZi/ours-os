#include <ours/panic.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ours/phys/console.hpp>
#include <ours/phys/aspace.hpp>

#include "../legacy-boot.hpp"

#include <gktl/static_objects.hpp>
#include <ustl/lazy_init.hpp>
#include <arch/system.hpp>

namespace ours::phys {
    LegacyBoot LegacyBoot::g_legacy_boot;

    NO_MANGLE NO_RETURN
    auto phys_main(PhysAddr loader_param) -> void
    {
        gktl::init_static_objects();
        init_early_console();
        println("loader params at 0x{:X}", loader_param);

        LegacyBoot::get().parse_params(loader_param);

        // Temporary entry
        obi_main(PhysAddr(LegacyBoot::get().ramdisk_.data()));

        panic("Never reach at here");
    }

} // ours::phys
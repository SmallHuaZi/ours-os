#include "legacy-boot.hpp"
#include "ours/panic.hpp"
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>

#include <lz4/lz4.hpp>
#include <gktl/static_objects.hpp>

namespace ours::phys {
    struct MainImageHeader
    {
        usize entry_point;
        usize reserved_size;
    };

    LegacyBoot LEGACY_BOOT{};

    NO_MANGLE NO_RETURN
    auto phys_main(usize loader_param) -> void
    {
        gktl::init_static_objects();
        init_early_console();
        println("loader params at {}", loader_param);

        LEGACY_BOOT.init_memory(loader_param, ASPACE);

        panic("Never reach at here.");
    }

} // ours::phys
#include "legacy-boot.hpp"
#include <ours/phys/init.hpp>

#include <lz4/lz4.hpp>

namespace ours::phys {
    struct MainImageHeader
    {
        usize entry_point;
        usize reserved_size;
    };

    NO_MANGLE
    auto phys_main(usize loader_param) -> void
    {
        LEGACY_BOOT.init_memory(loader_param, ASPACE);
    }

} // ours::phys
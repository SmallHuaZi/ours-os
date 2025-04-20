#include <gktl/init_hook.hpp>

#include <ustl/views/span.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace gktl {
    extern InitHook g_init_hook_start[] LINK_NAME("__init_hook_start");
    extern InitHook g_init_hook_end[] LINK_NAME("__init_hook_end");

    InitLevel g_init_level;
    InitLevel g_last_level;

    /// The hooks in range [level, level + delta) would be called.
    auto set_init_level(InitLevel level, usize delta) -> void {
        for (auto it = g_init_hook_start; it != g_init_hook_end; ++it) {
            if (it->level_ < g_last_level || it->level_ >= level + delta) {
                continue;
            }

            it->hook_();
        }

        g_init_level = ustl::algorithms::max(g_init_level, level);
        g_last_level = g_init_level + delta;
    }

} // namespace gktl
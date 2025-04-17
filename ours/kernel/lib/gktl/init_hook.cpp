#include <gktl/init_hook.hpp>

#include <ustl/views/span.hpp>
#include <ustl/algorithms/sort.hpp>

namespace gktl {
    extern InitHook g_init_hook_start[] LINK_NAME("__init_hook_start");
    extern InitHook g_init_hook_end[] LINK_NAME("__init_hook_end");

    /// The hooks in range [g_last_level, level) would be called.
    auto set_init_level(InitLevel level, usize delta) -> void {
        for (auto it = g_init_hook_start; it != g_init_hook_end; ++it) {
            if (it->level_ < level || it->level_ >= level + delta) {
                continue;
            }

            it->hook_();
        }
    }

} // namespace gktl
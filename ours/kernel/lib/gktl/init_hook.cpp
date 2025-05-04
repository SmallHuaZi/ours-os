#include <gktl/init_hook.hpp>

#include <ustl/views/span.hpp>
#include <ustl/algorithms/minmax.hpp>
#include <ustl/algorithms/sort.hpp>

namespace gktl {
    extern InitHook g_init_hook_start[] LINK_NAME("__init_hook_start");
    extern InitHook g_init_hook_end[] LINK_NAME("__init_hook_end");

    InitLevel g_init_level = InitLevel::LowestLevel;
    static ustl::views::Span<InitHook> s_init_hooks(g_init_hook_start, g_init_hook_end);

    /// The hooks in range [level, level + delta) would be called.
    auto set_init_level(InitLevel level, usize delta) -> void {
        if (g_init_level == InitLevel::LowestLevel) {
            ustl::algorithms::sort(s_init_hooks.begin(), s_init_hooks.end(),
                [](InitHook const &lhs, InitHook const &rhs) {
                    return lhs.level_ < rhs.level_;
                });
        }

        auto it = s_init_hooks.begin();
        for (auto end = s_init_hooks.end(); it != end; ++it) {
            if (it->level_ >= level + delta) {
                break;
            }

            it->hook_();
        }

        s_init_hooks = ustl::views::Span<InitHook>(it, s_init_hooks.end());
        g_init_level = ustl::algorithms::max(g_init_level, level);
    }

} // namespace gktl
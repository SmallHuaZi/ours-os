#include <ours/arch/thread.hpp>

namespace ours::task {
    NO_MANGLE
    auto x86_switch_context() -> void
    {}

    auto X86Thread::switch_context(Self &prev, Self &next) -> void
    {}

} // namespace ours::task
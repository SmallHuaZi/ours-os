#include <ours/panic.hpp>
#include <ours/phys/print.hpp>

namespace ours {
    NO_RETURN
    auto panic() -> void
    {
        phys::vprint("panic");
        while (1);
    }

    NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void
    {
        phys::vprint("panic", args);
        while (1);
    }
} // namespace ours
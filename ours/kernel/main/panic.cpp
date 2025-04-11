#include <ours/panic.hpp>

namespace ours {
    NO_RETURN
    auto panic() -> void { 
        while (1); 
    }

    NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void { 
        while (1); 
    }

} // namespace ours
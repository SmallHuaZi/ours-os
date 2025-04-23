#include <ours/panic.hpp>

#include <logz4/log.hpp>

namespace ours {
    NO_RETURN
    auto panic() -> void { 
        while (1); 
    }

    NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void { 
        log::do_log(logz4::get_global_logger(), logz4::Level::Error, fmt, args);
        while (1); 
    }

} // namespace ours
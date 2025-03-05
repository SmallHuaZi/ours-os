#include <logz4/log.hpp>

#include <ustl/fmt/format_to.hpp>

namespace logz4 {
    auto log_impl(Logger &logger, Level level, ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void
    {
        logger.log({ 
            .level = level,
            .args = args,
            .fmtstr = fmt,
        });
    }

    auto wlog(Logger &logger, Level level, ustl::views::WStringView fmt, ustl::fmt::FormatArgs const &args) -> void
    {

    }

} // namespace logz4
/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///

#ifndef LOGZ4_LOG_HPP
#define LOGZ4_LOG_HPP 1

#include <logz4/level.hpp>
#include <ustl/fmt/format_to.hpp>

namespace logz4 {
    auto init() -> void;

    template <typename... Args>
    using FormatString = ustl::fmt::FormatString<Args...>;

    template <typename... Args>
    auto log(Level level, FormatString<Args...> fmt, Args &&...args) -> void
    {
        // auto record = new Record();
        // if (!ustl::fmt::format_to(record.begin(), fmt, args)) {  }
    }

    template <typename... Args>
    inline auto debug(FormatString<Args...> fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Debug, fmt, ustl::forward(args)...);  }

    template <typename... Args>
    inline auto info(FormatString<Args...> fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Info, fmt, ustl::forward(args)...);  }

    template <typename... Args>
    inline auto trace(FormatString<Args...> fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Trace, fmt, ustl::forward(args)...);  }

    template <typename... Args>
    inline auto warn(FormatString<Args...> fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Warn, fmt, ustl::forward(args)...);  }

    template <typename... Args>
    inline auto error(FormatString<Args...> fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Status, fmt, ustl::forward(args)...);  }

} // namespace log

#define LOG_DEBUG(FMT, ...)     log::debug(FMT, __VA_ARGS__)
#define LOG_INFO(FMT, ...)      log::info(FMT, __VA_ARGS__)
#define LOG_TRACE(FMT, ...)     log::trace(FMT, __VA_ARGS__)
#define LOG_WARN(FMT, ...)      log::warn(FMT, __VA_ARGS__)
#define LOG_ERROR(FMT, ...)     log::error(FMT, __VA_ARGS__)

/// 
namespace log = logz4;

#endif // #ifndef LOGZ4_LOG_HPP_
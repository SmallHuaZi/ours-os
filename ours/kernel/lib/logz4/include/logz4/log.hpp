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
#include <logz4/logger.hpp>

#include <ustl/fmt/make_format_args.hpp>

namespace logz4 {
    auto init(Logger *logger) -> void;

    auto register_logger(Logger &logger) -> void;

    auto unregister_logger(Logger &logger) -> void;

    auto get_global_logger() -> Logger &;

    auto set_global_logger(Logger &logger) -> void;

    auto log_impl(Logger &logger, Level level, ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void;

    auto wlog_impl(Logger &logger, Level level, ustl::views::WStringView fmt, ustl::fmt::FormatArgs const &args) -> void;

    template <typename... Args>
    auto log(Logger &logger, Level level, ustl::views::StringView fmt, Args &&...args) -> void
    {  return log_impl(logger, level, fmt, ustl::fmt::make_format_args(args...));  }

    template <typename... Args>
    auto wlog(Logger &logger, Level level, ustl::views::WStringView fmt, Args &&...args) -> void
    {  return wlog_impl(logger, level, fmt, ustl::fmt::make_format_args(args...));  }

    template <typename... Args>
    inline auto debug(Logger &logger, ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(logger, Level::Debug, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto info(Logger &logger, ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(logger, Level::Info, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto trace(Logger &logger, ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(logger, Level::Trace, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto warn(Logger &logger, ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(logger, Level::Warn, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto error(Logger &logger, ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(logger, Level::Error, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    auto log(Level level, ustl::views::StringView fmt, Args &&...args) -> void
    {  return log(get_global_logger(), level, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto debug(ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Debug, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto info(ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Info, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto trace(ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Info, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto warn(ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Error, fmt, ustl::forward<Args>(args)...);  }

    template <typename... Args>
    inline auto error(ustl::views::StringView fmt, Args &&...args) -> void
    {  return logz4::log<Args...>(Level::Error, fmt, ustl::forward<Args>(args)...);  }

} // namespace logz4

/// 
namespace log = logz4;

#endif // #ifndef LOGZ4_LOG_HPP_
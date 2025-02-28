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

#ifndef LOGZ4_LOGGER_HPP
#define LOGZ4_LOGGER_HPP 1

#include <logz4/level.hpp>
#include <ustl/source_location.hpp>

namespace logz4 {
    struct Record
    {
        Level level;
        char const *target;
        ustl::SourceLocation location;
    };

    class Logger
    {
    public:
        Logger();

        virtual ~Logger();

        virtual auto is_enabled() -> bool
        {  return true;  }

        virtual auto log(Record const &record) -> void = 0;

        virtual auto flush() -> void = 0;

        virtual auto kill() -> void = 0;
    };

} // namespace logz4

#endif // #ifndef LOGZ4_LOGGER_HPP
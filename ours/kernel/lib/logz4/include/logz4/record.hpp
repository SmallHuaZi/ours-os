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

#ifndef LOGZ4_RECORD_HPP
#define LOGZ4_RECORD_HPP 1

#include <logz4/level.hpp>

namespace logz4 {
    class Record
    {
    private:
        Level level_;
        char const *target_;
    };
}

#endif // #ifndef LOGZ4_RECORD_HPP
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
#ifndef OURS_STATUS_HPP
#define OURS_STATUS_HPP 1

namespace ours {
    enum class Status {
        Ok,
        Unimplemented,
        Unsupported,
        Unreachable,
        Error,
        Fail,
        OutOfMem,
        OutOfRange,
        InvalidArguments,
        InternalError,
        BadState,
        TimeOut,
        NotFound,
        ShouldWait,
        MisAligned,
        AlreadyExists,
        NoResource,
        NoCapability,
        MaxCount,
    };

    inline auto to_string(Status error) -> char const * {
        typedef unsigned int    StatusVal;

        char const *to_string[] = {
            "Ok",
            "Fail",
            "Out of memory",
            "Invalid arguments",
            "Unimplemented",
            "Unreachable",
            "Error",
        };

        if (StatusVal(error) > StatusVal(Status::MaxCount)) {
            return "unknown error";
        }

        return to_string[StatusVal(error)];
    }

} // namespace ours

#endif // #ifndef OURS_STATUS_HPP
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

#ifndef USTL_FMT_ERROR_HPP
#define USTL_FMT_ERROR_HPP 1

#include <ustl/result.hpp>

namespace ustl::fmt {
    enum class FmtErr {
        TerminatesAtLeftBrace, // Terminate at a '{'
        MissingRightBrace, // Terminate at a '{'
        ArgsIndexOutOfRange, // Terminate at a '{'
        InvalidEscapeSequence,
        ExpectedRightBraceButNot,
        Validate,
        InvalidArgIndex,
    };

    template <typename T> 
    using Result = Result<T, FmtErr>;
}

#endif // #ifndef USTL_FMT_ERROR_HPP
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

#ifndef USTL_FMT_FORMATTER_HPP
#define USTL_FMT_FORMATTER_HPP 1

#include <format>

namespace ustl::fmt {
    template <typename T, typename Char = char>
    using Formatter = std::formatter<T, Char>;

} // namespace ustl::fmt

#endif // #ifndef USTL_FMT_FORMATTER_HPP
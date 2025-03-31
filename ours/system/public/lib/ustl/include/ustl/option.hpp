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
#ifndef USTL_OPTION_HPP
#define USTL_OPTION_HPP 1

#include <optional>

namespace ustl {
    template <typename T>
    struct Option: std::optional<T> {
        typedef std::optional<T>    Base;
        using Base::Base;

        typedef typename Base::value_type   Element;
    };

    constexpr auto NONE = std::nullopt;

    constexpr auto none() {
        return std::nullopt;
    }

    template <typename T>
    constexpr inline auto some(T &t) -> Option<std::remove_reference_t<T>> {
        return { t };
    }

    template <typename T>
    constexpr inline auto some(T &&t) -> Option<std::remove_reference_t<T>> {
        return { std::move(t) };
    }

} // namespace ustl

#endif // #ifndef USTL_OPTION_HPP
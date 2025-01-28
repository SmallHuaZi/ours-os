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

#ifndef GKTL_CANARY_HPP
#define GKTL_CANARY_HPP

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/traits/char_traits.hpp>

namespace gktl {
    CXX20_CONSTEVAL
    auto magic(char const *literal) -> ours::u32
    {
        auto const n = ustl::traits::CharTraits<char>::length(literal);
        auto res = 0;
        for (auto i = 0; i < n; ++i) {
            res = (res << 8) + literal[i];
        }
        return res;
    }

    template <ours::u32 Magic>
    struct Canary
    {
        CXX11_CONSTEXPR
        Canary()
            : magic_(Magic)
        {}

        auto verify() const -> void
        {
            this->magic_ == Magic;
        }

        ours::u32 const magic_;
    };

} // namespace gktl

/// 
#define GKTL_CANARY(TYPE, FIELD_NAME) ::gktl::Canary<::gktl::magic(#TYPE)> const FIELD_NAME;

#endif // GKTL_CANARY_HPP
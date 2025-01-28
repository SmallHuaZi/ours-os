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

#ifndef USTL_VARIANT_HPP
#define USTL_VARIANT_HPP 1

#include <variant>

namespace ustl {
    using std::get;
    using std::variant_size;
    using std::variant_alternative;

    template <typename... Types>
    using Variant = std::variant<Types...>;

} // namespace ustl

#endif // USTL_VARIANT_HPP
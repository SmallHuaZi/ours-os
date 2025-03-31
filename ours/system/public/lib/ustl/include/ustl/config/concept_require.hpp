// ustl/config USTL/CONFIG_CONCEPT_REQUIRE_HPP
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

#ifndef USTL_CONFIG_CONCEPT_REQUIRE_HPP
#define USTL_CONFIG_CONCEPT_REQUIRE_HPP 1

#ifdef USTL_ENABLE_CONCEPT_REQUIRE
#   include <ustl/type_traits.hpp>
#   define USTL_REQUIRES(...)   \
        static_assert(__VA_ARGS__, __FILE__ " at line"__LINE__":"\
        " the template arguments doesn't satisfy the"\
        " concept expression ["#__VA_ARGS__ "]")
#else
#   define USTL_REQUIRES(...)
#endif //#ifdef USTL_ENABLE_CONCEPT_REQUIRE

#endif // #ifndef USTL_CONFIG_CONCEPT_REQUIRE_HPP
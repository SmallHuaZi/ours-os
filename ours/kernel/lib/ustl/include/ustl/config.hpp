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

#ifndef USTL_CONFIG_HPP
#define USTL_CONFIG_HPP 1

#if defined(__cplusplus) && __cplusplus < 201103L
#error "The C++ version requires a minimum of 11 in ustl"
#endif

#define USTLCXX 1

#include <ustl/config/core.hpp>
#include <ustl/config/types.hpp>
#include <ustl/config/lang_items.hpp>
#include <ustl/config/assert.hpp>

#include <ustl/config/namespace.hpp>

#define USTL    ::ustl::

#endif // #ifndef USTL_CONFIG_HPP
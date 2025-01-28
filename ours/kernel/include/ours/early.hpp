// ours OURS_EARLY_HPP
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

#ifndef OURS_EARLY_HPP
#define OURS_EARLY_HPP 1

#include <ours/config.hpp>

#define EARLY_SECTION(name) LINK_SECTION(".early." name)

#define EARLY_CODE      EARLY_SECTION("code")
#define EARLY_DATA      EARLY_SECTION("data")
#define EARLY_RODATA    EARLY_SECTION("rodata")

#endif // #ifndef OURS_EARLY_HPP
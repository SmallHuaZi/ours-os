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

#ifndef OURS_INIT_HPP
#define OURS_INIT_HPP 1

#include <ours/config.hpp>

#define INIT_SECTION(name) LINK_SECTION(".init." name)

/// The code marked this just called once on boot time.
#define INIT_CODE      INIT_SECTION("code")

/// The data marked this just used on boot time.
#define INIT_DATA      INIT_SECTION("data")

/// The read only data marked this just read on boot time.
#define INIT_CONST     INIT_SECTION("rodata")

#endif // #ifndef OURS_INIT_HPP
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

#ifndef OMI_ASM_HPP
#define OMI_ASM_HPP 1

#include <ours/asm.hpp>
#include <omi/defines.hpp>

#define OMI_HEADER(NAME, LENGTH, ...) \
    DATA(NAME) \
        .int    LENGTH;\
        .int    0; \
        .int    OMI_HEADER_MAGIC;\
        .int    0;\
    DATA_END(NAME)

#endif // #ifndef OMI_ASM_HPP
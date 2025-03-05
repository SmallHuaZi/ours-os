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

#ifndef OURS_ASM_HPP
#define OURS_ASM_HPP 1

#if __has_include(<ours/arch/asm.hpp>)
#   include <ours/arch/asm.hpp>
#endif

/// For functions that don't have an "end" or don't want .cfi_startproc
#define LOCAL_FUNCTION_NOCFI(x)     .type x,STT_FUNC; x:
#define FUNCTION_NOCFI(x)           .global x; .hidden x; LOCAL_FUNCTION_NOCFI(x)

#define LOCAL_FUNCTION(x)   LOCAL_FUNCTION_NOCFI(x) .cfi_startproc
#define FUNCTION(x)         .global x; .hidden x; LOCAL_FUNCTION(x)
#define FUNCTION_END(x)     .cfi_endproc; .size x, . - x

#define LOCAL_DATA(x)   .type x,STT_OBJECT; x:
#define DATA(x)         .global x; .hidden x; LOCAL_DATA(x)
#define DATA_END(x)     .size x, . - x

#define SIZE(...)       .quad  __VA_ARGS__

#endif // #ifndef OURS_ASM_HPP
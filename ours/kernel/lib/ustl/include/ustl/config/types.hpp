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

#ifndef USTL_CONFIG_TYPES_HPP
#define USTL_CONFIG_TYPES_HPP 1

#include <cstddef>
#include <cstdint>

namespace ustl {
    typedef ::int8_t     i8;
    typedef ::int16_t    i16;
    typedef ::int32_t    i32;
    typedef ::int64_t    i64;
    typedef ::ptrdiff_t  isize;
    
    typedef ::uint8_t     u8;
    typedef ::uint16_t    u16;
    typedef ::uint32_t    u32;
    typedef ::uint64_t    u64;
    typedef ::size_t      usize;

#ifdef __STDCPP_FLOAT16_T__
    typedef ::std::float16_t    f16;
    typedef ::std::float32_t    f32;
    typedef ::std::float64_t    f64;
#else
    typedef struct F16 {}    f16;
    typedef struct F32 {}    f32;
    typedef struct F64 {}    f64;
#endif // #ifdef __STDCPP_FLOAT16_T__

} // namespace ustl

#endif // #ifndef USTL_CONFIG_TYPES_HPP
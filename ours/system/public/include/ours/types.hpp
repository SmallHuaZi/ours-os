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

#ifndef OURS_TYPES_HPP
#define OURS_TYPES_HPP 1

#include <cstddef>
#include <cstdint>

namespace ours {
    // typedef ::ustl::i8        i8;
    // typedef ::ustl::i16       i16;
    // typedef ::ustl::i32       i32;
    // typedef ::ustl::i64       i64;
    // typedef ::ustl::isize     isize;

    // typedef ::ustl::u8        u8;
    // typedef ::ustl::u16       u16;
    // typedef ::ustl::u32       u32;
    // typedef ::ustl::u64       u64;
    // typedef ::ustl::usize     usize;
    typedef int8_t      i8;
    typedef int16_t     i16;
    typedef int32_t     i32;
    typedef int64_t     i64;
    typedef ptrdiff_t   isize;

    typedef uint8_t     u8;
    typedef uint16_t    u16;
    typedef uint32_t    u32;
    typedef uint64_t    u64;
    typedef size_t      usize;

    /// For type check.
    typedef struct { usize inner_; } Handle;
    typedef struct { usize inner_; } Signal;
    typedef struct { usize inner_; } KoId;

} // namespace ours

#endif // #ifndef OURS_TYPES_HPP
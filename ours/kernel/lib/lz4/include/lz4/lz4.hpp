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

#ifndef LZ4_LZ4_HPP
#define LZ4_LZ4_HPP 1

#include <ours/types.hpp>

namespace lz4 {
    using ours::isize;
    using ours::usize;

    auto compress(void const *src, void *dest, usize src_size, usize dest_size) -> isize;

    auto decompress(void const *src, void *dest, usize src_size, usize dest_size) -> isize;

    auto calc_uncompressed_size(void const *src, usize size) -> usize;

} // namespace lz4

#endif // #ifndef LZ4_LZ4_HPP
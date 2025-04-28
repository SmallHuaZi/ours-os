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
#ifndef KTL_VEC_HPP
#define KTL_VEC_HPP 1

#include <ktl/allocator.hpp>
#include <ustl/collections/vec.hpp>

namespace ktl {
    template <typename T, typename A = ktl::Allocator<T>>
    using Vec = ::ustl::collections::Vec<T, A>;

} // namespace ktl

#endif // #ifndef KTL_VEC_HPP
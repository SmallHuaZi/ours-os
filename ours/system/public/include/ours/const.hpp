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
#ifndef OURS_CONST_HPP
#define OURS_CONST_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ustl/limits.hpp>

namespace ours {
    CXX11_CONSTEXPR
    static auto const kMaxVirtAddrBit = ustl::NumericLimits<VirtAddr>::DIGITS;

} // namespace ours

#endif // #ifndef OURS_CONST_HPP
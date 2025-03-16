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

#ifndef KTL_RESULT_HPP
#define KTL_RESULT_HPP 1

#include <ours/status.hpp>
#include <ustl/result.hpp>

namespace ktl {
    template <typename T>
    using Result = ustl::Result<T, ours::Status>;

    using ustl::ok; 
    using ustl::err;

} // namespace ktl

#endif // #ifndef KTL_RESULT_HPP
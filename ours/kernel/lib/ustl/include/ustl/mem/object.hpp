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

#ifndef USTL_MEM_OBJECT_HPP
#define USTL_MEM_OBJECT_HPP 1

#include <memory>

namespace ustl::mem {
    using std::construct_at;
    using std::destroy_at;
    using std::destroy_n;
    using std::destroy;

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_OBJECT_HPP
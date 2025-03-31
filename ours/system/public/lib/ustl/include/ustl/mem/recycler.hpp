// ustl USTL_RECYCLER_HPP
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

#ifndef USTL_MEM_RECYCLER_HPP
#define USTL_MEM_RECYCLER_HPP 1

namespace ustl::mem {
    template <typename T>
    struct Recycler
    {
        static auto dispose(T *ptr) -> void
        {}

        static auto destroy(T *ptr) -> void
        {  delete ptr;  }
    };

} // namespace ustl::mem

#endif // #ifndef USTL_RECYCLER_HPP
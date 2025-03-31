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

#ifndef USTL_LAZY_INIT_HPP
#define USTL_LAZY_INIT_HPP 1

#include <ustl/util/move.hpp>

namespace ustl {
    template <typename T, int Alignment = alignof(T)>
    union LazyInit
    {
        template <typename... Args> 
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto init(Args&&... args) -> T *
        {  return new (data()) T(ustl::forward<Args>(args)...);  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto data() -> T *
        {  return reinterpret_cast<T *>(data_); }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator->() -> T *
        {  return this->data();  }

    private:
        char data_[sizeof(T)];
    };

} // namespace ustl

#endif // #ifndef USTL_LAZY_INIT_HPP
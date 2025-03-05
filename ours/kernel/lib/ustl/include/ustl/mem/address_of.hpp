#ifndef USTL_MEM_ADDRESS_OF_HPP
#define USTL_MEM_ADDRESS_OF_HPP 1

#include <ustl/config.hpp>

namespace ustl::mem {
    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto address_of(T &t) USTL_NOEXCEPT ->  T *
    {  return &t;  }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto address_of(T const &t) USTL_NOEXCEPT ->  T const *
    {  return &t;  }

} // namespace ustl::mem

#endif // #ifndef USTL_MEM_ADDRESS_OF_HPP
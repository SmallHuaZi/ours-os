#ifndef USTL_UTIL_ARITHMETIC_HPP
#define USTL_UTIL_ARITHMETIC_HPP 1

#include <ustl/config.hpp>

#include <ustl/traits/declval.hpp>
#include <ustl/traits/enable_if.hpp>
#include <ustl/traits/make_void.hpp>
#include <ustl/traits/underlying_type.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace ustl::util {
    template <typename T>
    struct Arithmeticible
    {
        T arithmeticible_value_;
    };

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator+(Arithmeticible<T> a, Arithmeticible<T> b) -> ustl::util::EnableIfArithmeticible<T>
    {
        typedef ustl::traits::UnderlyingTypeT<T>  U;
        return static_cast<T>(static_cast<U>(a) + static_cast<U>(b));
    }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator-(T a, T b) -> ustl::util::EnableIfArithmeticible<T>
    {
        typedef ustl::traits::UnderlyingTypeT<T>  U;
        return static_cast<T>(static_cast<U>(a) - static_cast<U>(b));
    }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator*(T a, T b) -> ustl::util::EnableIfArithmeticible<T>
    {
        typedef ustl::traits::UnderlyingTypeT<T>  U;
        return static_cast<T>(static_cast<U>(a) ^ static_cast<U>(b));
    }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator|=(T &a, T b) -> ustl::util::EnableIfArithmeticible<T> &
    {
        a = a | b;
        return a;
    }

    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator&=(T &a, T b) -> ustl::util::EnableIfArithmeticible<T> &
    {
        a = a & b;
        return a;
    }
    template <typename T>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto operator^=(T &a, T b) -> ustl::util::EnableIfArithmeticible<T> &
    {
        a = a ^ b;
        return a;
    }

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_ARITHMETIC_HPP
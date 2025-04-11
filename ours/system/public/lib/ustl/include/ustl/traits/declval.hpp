#ifndef USTL_UTIL_DECLVAL_HPP
#define USTL_UTIL_DECLVAL_HPP

namespace ustl::traits {
    template <typename T>
    static constexpr auto declval() -> T;
}

#endif // USTL_UTIL_DECLVAL_HPP

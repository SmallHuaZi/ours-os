#ifndef USTL_TRAITS_DECAY_HPP
#define USTL_TRAITS_DECAY_HPP

#include <type_traits>

namespace ustl::traits {
    template <typename T>
    using DecayT = ::std::decay_t<T>;

} // namespace ustl::traits

#endif // USTL_TRAITS_DECAY_HPP

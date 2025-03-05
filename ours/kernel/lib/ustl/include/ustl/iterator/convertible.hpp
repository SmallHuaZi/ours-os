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

#ifndef USTL_ITERATOR_CONVERTIBLE_HPP
#define USTL_ITERATOR_CONVERTIBLE_HPP 1

#include <ustl/iterator/traits.hpp>
#include <ustl/traits/is_convertible.hpp>

namespace ustl::iter {
    template <typename IterFrom, typename IterTo>
    struct IsConvertible
    {
        typedef IterTraits<IterFrom>  IterTraits1;
        typedef IterTraits<IterTo>    IterTraits2;
        typedef typename IterTraits1::Element   From;
        typedef typename IterTraits2::Element   To;

        USTL_CONSTEXPR 
        static bool const VALUE = traits::IsConvertible<From, To>::VALUE;
    };

    template <typename IterFrom, typename IterTo>
    USTL_CONSTEXPR 
    bool const IsConvertibleV = IsConvertible<IterFrom, IterTo>::VALUE; 

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_CONVERTIBLE_HPP
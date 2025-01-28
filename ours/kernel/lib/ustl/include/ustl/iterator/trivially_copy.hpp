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

#ifndef USTL_ITERATOR_TRIVIALLY_COPY_HPP
#define USTL_ITERATOR_TRIVIALLY_COPY_HPP 1

#include <ustl/iterator/traits.hpp>
#include <ustl/traits/is_trivially_copy_constructible.hpp>

namespace ustl::iter {
    template <typename Iterator>
    struct IsTriviallyCopyConstructible
    {
        typedef IterTraits<Iterator>       Traits;
        typedef typename Traits::Element   Element;

        USTL_CONSTEXPR 
        static bool const VALUE = traits::IsTriviallyCopyConstructible<Element>::VALUE;       
    };
    
    template <typename T>
    USTL_CONSTEXPR 
    bool const IsTriviallyCopyConstructibleV = IsTriviallyCopyConstructible<T>::VALUE;
    
} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_TRIVIALLY_COPY_HPP
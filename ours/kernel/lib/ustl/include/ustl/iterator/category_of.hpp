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

#ifndef USTL_ITERATOR_CATEGORY_OF_HPP
#define USTL_ITERATOR_CATEGORY_OF_HPP 1

#include <ustl/iterator/is_iter.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl::iter {
    template <typename... Iterators>
    struct IterCategoryOf;

    template <typename Iterator>
    struct IterCategoryOf<Iterator>
    {
        typedef typename IterTraits<Iterator>::Category     Type;
    };

    template <typename Iterator1, typename... Iterators>
    struct IterCategoryOf<Iterator1, Iterators...>
    {
        typedef typename IterCategoryOf<Iterator1>::Type    Category1;
        typedef typename IterCategoryOf<Iterators...>::Type Category2;
        typedef traits::ConditionalT<traits::IsBaseOfV<Category1, Category2>, Category1, Category2>
                Type;
    };

    template <typename... Iterators>
    using IterCategoryOfT = typename IterCategoryOf<Iterators...>::Type;
    
    template <typename... Iterators>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto category_of() USTL_NOEXCEPT -> IterCategoryOfT<Iterators...>
    {  
        return IterCategoryOfT<Iterators...>();  
    }

} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_CATEGORY_OF_HPP
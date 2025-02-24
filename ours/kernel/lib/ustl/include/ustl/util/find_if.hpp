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

#ifndef USTL_UTIL_FIND_IF_HPP
#define USTL_UTIL_FIND_IF_HPP 1

#include <ustl/traits/conditional.hpp>
#include <ustl/placeholders.hpp>

namespace ustl::util {
    /// FindIf<Matcher, ObjectList>
    /// Return `Monostate` by default if not found, otherwise return the type matched.
    template <template <typename Object> typename Matcher,
              typename... ObjectList>
    struct FindIf;

    template <template <typename Object> typename Matcher>
    struct FindIf<Matcher>
    {  typedef Monostate    Type;  };

    template <template <typename Object> typename Matcher,
              typename HeadObject,
              typename... ObjectList> 
    struct FindIf<Matcher, HeadObject, ObjectList...>
    {
        typedef traits::ConditionalT<
            Matcher<HeadObject>::VALUE, 
            HeadObject, 
            typename FindIf<Matcher, ObjectList...>::Type
        > Type;
    };

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_FIND_IF_HPP
// ustl/traits USTL/TRAITS_TYPE_DETECTOR_HPP
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

#ifndef USTL_TRAITS_TYPE_DETECTOR_HPP
#define USTL_TRAITS_TYPE_DETECTOR_HPP 1

#include <ustl/traits/idextity.hpp>
#include <ustl/traits/conditional.hpp>

#define USTL_MPL_CREATE_TYPE_DETECTOR(TYPE)                                             \
    template <typename Class>                                                           \
    struct HasType##TYPE                                                                \
    {                                                                                   \
        struct FoundTag { char _[4]; };                                                 \
        struct NotFoundTag  { char _[8]; };                                             \
                                                                                        \
        template <typename U>                                                           \
        static FoundTag test(typename U::Type *, int);                                  \
                                                                                        \
        template <typename U>                                                           \
        static NotFoundTag test(...);                                                   \
                                                                                        \
    public:                                                                             \
        static const bool VALUE = sizeof(test<Class>(0, 0)) == sizeof(FoundTag);        \
    };                                                                                  \
    template <typename T, typename DefaultType>                                         \
    struct Get ## TYPE ## OrDefaultType                                                 \
    {                                                                                   \
        struct DefaultTypeWrapper {                                                     \
            typedef DefaultType TYPE;                                                   \
        };                                                                              \
        typedef typename ustl::traits::ConditionalT<                                    \
            HasType ## TYPE<T>::VALUE, T, DefaultTypeWrapper                            \
        >::TYPE  TYPE;                                                         \
    }; 

#endif // #ifndef USTL_TRAITS_TYPE_DETECTOR_HPP
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

#ifndef USTL_UTIL_PACK_OPTIONS_HPP
#define USTL_UTIL_PACK_OPTIONS_HPP 1

#include <ustl/util/types_list.hpp>

#define USTL_TYPE_OPTION(OPTION_NAME, TYPEDEF_NAME)                  \
template<typename T>                                                            \
struct OPTION_NAME                                                              \
{                                                                               \
   template<typename Base>                                                      \
   struct Pack : Base                                                           \
   {                                                                            \
      typedef T TYPEDEF_NAME;                                                   \
   };                                                                           \
};

#define USTL_CONSTANT_OPTION(OPTION_NAME, TYPE, CONSTANT_NAME)   \
template<TYPE VAL>                                                          \
struct OPTION_NAME                                                          \
{                                                                           \
   static const TYPE VALUE = VAL;                                           \
                                                                            \
   template<typename Base>                                                  \
   struct Pack: Base                                                        \
   {                                                                        \
      static const TYPE CONSTANT_NAME = VAL;                                \
   };                                                                       \
};

namespace ustl {
    template <typename TypesList>
    struct DoPackOptions;

    // Only default options.
    template <typename DefaultOption>
    struct DoPackOptions<TypesList<DefaultOption>>
    {
        typedef DefaultOption  Type;
    };

    // Pack two options, `Option1` has higher priority than `Option2`.
    template <typename Option1, typename Option2>
    struct DoPackOptions<TypesList<Option1, Option2>>
    {
        typedef typename Option1::template Pack<Option2>   Type;
    };

    // Remove unable options.
    template <typename... Options>
    struct DoPackOptions<TypesList<void, Options...>>
    {
        typedef typename DoPackOptions<TypesList<Options...>>::Type  Type;
    };

    // Do pack.
    template <typename Option, typename... OtherOptions>    
    struct DoPackOptions<TypesList<Option, OtherOptions...>>
    {
        typedef typename Option::template Pack<
            typename DoPackOptions<TypesList<OtherOptions...>>::Type>   Type;
    };


    //! @brief Custom attribute packaging by inheriting subclass attributes 
    //!        with higher priority than basic attributes.
    //! @tparam DefaultOptions The default, which is the configuration option 
    //!         with the lowest priority.
    //! @tparam CustomOptions  User-specified, high-priority options
    template <typename DefaultOptions, typename... CustomOptions>    
    struct PackOptions
    { 
        typedef TypesList<DefaultOptions, CustomOptions...>   OptionsList;
        typedef InvertTypesListT<OptionsList>  InvertedOptionsList; 
        typedef typename DoPackOptions<InvertedOptionsList>::Type  Type;
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_PACK_OPTIONS_HPP
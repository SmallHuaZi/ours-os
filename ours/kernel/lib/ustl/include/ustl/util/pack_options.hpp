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
template<TYPE VAL>                                               \
struct OPTION_NAME                                               \
{                                                                \
    USTL_CONSTEXPR                                               \
    static const TYPE VALUE = VAL;                               \
                                                                 \
    template<typename Base>                                      \
    struct Pack: Base                                            \
    {                                                            \
        USTL_CONSTEXPR                                           \
        static const TYPE CONSTANT_NAME = VAL;                   \
    };                                                           \
};

namespace ustl {
    template <typename TypeList>
    struct DoPackOptions;

    // Only default options.
    template <typename DefaultOption>
    struct DoPackOptions<TypeList<DefaultOption>>
    {  typedef DefaultOption  Type;  };

    // Pack two options, `Option1` has higher priority than `Option2`.
    template <typename Option1, typename Option2>
    struct DoPackOptions<TypeList<Option1, Option2>>
    {  typedef typename Option1::template Pack<Option2>   Type;  };

    // Remove unable options.
    template <typename... Options>
    struct DoPackOptions<TypeList<void, Options...>>
    {  typedef typename DoPackOptions<TypeList<Options...>>::Type  Type;  };

    // Do pack.
    template <typename Option, typename... OtherOptions>    
    struct DoPackOptions<TypeList<Option, OtherOptions...>>
    {
        typedef typename Option::template Pack<
            typename DoPackOptions<TypeList<OtherOptions...>>::Type
        > Type;
    };

    /// `PackOptions` 
    ///
    /// `CustomOptions` should be user-specified, and it is higher priority than `DefaultOptions`.
    /// `DefaultOptions` is used to give out the default configuration, usually open to lib-developer.
    ///
    /// The effect as the following code showed:
    ///  ```cpp 
    /// struct DefaultHttpConfig
    /// {
    ///     enum { Port = 7890 };
    ///     enum { MaxNumConnections = 32 };
    /// };
    /// 
    /// struct MyHttpConfig
    /// {
    ///     enum { MaxNumConnections = 64 };
    /// };
    ///
    /// typedef typename PackOptions<DefaultHttpConfig, MyHttpConfig>::Type PackedOptions;
    /// static_assert(PackedOption == 64); // It will pass compile
    /// static_assert(PackedOption == 32); // static assert error.
    ///
    template <typename DefaultOptions, typename... CustomOptions>
    struct PackOptions
    { 
        typedef TypeList<DefaultOptions, CustomOptions...>   OptionsList;
        typedef TypeListInvertT<OptionsList>  OptionsListInverted; 
        typedef typename DoPackOptions<OptionsListInverted>::Type  Type;
    };

} // namespace ustl

#endif // #ifndef USTL_UTIL_PACK_OPTIONS_HPP
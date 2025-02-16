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

#ifndef USTL_TRAITS_FUNCTION_DETECTOR_HPP
#define USTL_TRAITS_FUNCTION_DETECTOR_HPP 1

#define USTL_MPL_CREATE_METHOD_DETECTOR(Identifier, InstantiationKey)                   \
    template <typename T>                                                               \
    struct HasFn##InstantiationKey;                                                     \
                                                                                        \
    template <typename Class, typename Return, typename... Args>                        \
    struct HasFn##InstantiationKey<Return (Class::*)(Args...)>                          \
    {                                                                                   \
        typedef Class   ClassType;                                                      \
        typedef Return  ReturnType;                                                     \
                                                                                        \
        struct FoundTag { char _[4]; };                                                 \
        struct NotFoundTag  { char _[8]; };                                             \
                                                                                        \
        template <Return (Class::*)(Args...)>                                           \
        struct Receiver;                                                                \
                                                                                        \
        template <typename U>                                                           \
        static FoundTag test(Receiver<&U::Identifier> *, int);                          \
                                                                                        \
        template <typename U>                                                           \
        static NotFoundTag test(...);                                                   \
                                                                                        \
    public:                                                                             \
        static const bool VALUE = sizeof(test<Class>(0, 0)) == sizeof(FoundTag);        \
    };                                                                                  \
                                                                                        \
    template <typename Class, typename Return, typename... Args>                        \
    struct HasFn##InstantiationKey<Return (Class::*)(Args...) &>                        \
    {                                                                                   \
        typedef Class   ClassType;                                                      \
        typedef Return  ReturnType;                                                     \
                                                                                        \
        struct FoundTag { char _[4]; };                                                 \
        struct NotFoundTag  { char _[8]; };                                             \
                                                                                        \
        template <Return (Class::*)(Args...) &>                                         \
        struct Receiver;                                                                \
                                                                                        \
        template <typename U>                                                           \
        static FoundTag test(Receiver<&U::Identifier> *, int);                          \
                                                                                        \
        template <typename U>                                                           \
        static NotFoundTag test(...);                                                   \
                                                                                        \
    public:                                                                             \
        static const bool VALUE = sizeof(test<Class>(0, 0)) == sizeof(FoundTag);        \
    };                                                                                  \
                                                                                        \
    template <typename Class, typename Return, typename... Args>                        \
    struct HasFn##InstantiationKey<Return (Class::*)(Args...) const>                    \
    {                                                                                   \
        typedef Class   ClassType;                                                      \
        typedef Return  ReturnType;                                                     \
                                                                                        \
        struct FoundTag { char _[4]; };                                                 \
        struct NotFoundTag  { char _[8]; };                                             \
                                                                                        \
        template <Return (Class::*)(Args...) const>                                     \
        struct Receiver;                                                                \
                                                                                        \
        template <typename U>                                                           \
        static FoundTag test(Receiver<&U::Identifier> *, int);                          \
                                                                                        \
        template <typename U>                                                           \
        static NotFoundTag test(...);                                                   \
                                                                                        \
    public:                                                                             \
        static const bool VALUE = sizeof(test<Class>(0, 0)) == sizeof(FoundTag);        \
    };                                                                                  \
                                                                                        \
    template <typename Class, typename Return, typename... Args>                        \
    struct HasFn##InstantiationKey<Return (Class::*)(Args...) const &>                  \
    {                                                                                   \
        typedef Class   ClassType;                                                      \
        typedef Return  ReturnType;                                                     \
                                                                                        \
        struct FoundTag { char _[4]; };                                                 \
        struct NotFoundTag  { char _[8]; };                                             \
                                                                                        \
        template <Return (Class::*)(Args...) const &>                                   \
        struct Receiver;                                                                \
                                                                                        \
        template <typename U>                                                           \
        static FoundTag test(Receiver<&U::Identifier> *, int);                          \
                                                                                        \
        template <typename U>                                                           \
        static NotFoundTag test(...);                                                   \
                                                                                        \
    public:                                                                             \
        static const bool VALUE = sizeof(test<Class>(0, 0)) == sizeof(FoundTag);        \
    };

namespace ustl::traits {
    USTL_MPL_CREATE_METHOD_DETECTOR(operator-, Sub);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator-=, SubAssign);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator+, Add);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator+=, AddAssign);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator*, Mul);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator*=, MulAssign);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator/, Div);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator/=, DivAssign);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator%, Rem);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator%=, Rbootmemssign);

    USTL_MPL_CREATE_METHOD_DETECTOR(operator++, Inc);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator--, Dec);

    USTL_MPL_CREATE_METHOD_DETECTOR(operator*,  Def);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator->, Arrow);

    USTL_MPL_CREATE_METHOD_DETECTOR(operator[], Index);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator<=, LesserEqual);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator>=, GreaterEqual);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator<, Lesser);
    USTL_MPL_CREATE_METHOD_DETECTOR(operator>, Greater);

} // namespace ustl::traits

#endif // #ifndef USTL_TRAITS_FUNCTION_DETECTOR_HPP

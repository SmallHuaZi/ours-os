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

#ifndef USTL_CONFIG_LANG_ITEMS_HPP
#define USTL_CONFIG_LANG_ITEMS_HPP 1

#if __cplusplus > 201103ull
#   define USTL_CXX11_INLINE     inline
#   define USTL_CXX11_CONSTEXPR  constexpr
#else
#   define USTL_CXX11_INLINE
#   define USTL_CXX11_CONSTEXPR
#endif // #if __cplusplus > 201103ull

#if __cplusplus > 201703ull
#   define USTL_CXX17_INLINE     inline
#   define USTL_CXX17_CONSTEXPR  constexpr
#else
#   define USTL_CXX17_INLINE
#   define USTL_CXX17_CONSTEXPR
#endif // #if __cplusplus > 201703ull

#if __cplusplus > 202003ull
#   define USTL_CXX20_INLINE     inline
#   define USTL_CXX20_CONSTEXPR  constexpr
#   define USTL_CXX20_CONSTEVAL  consteval
#   define USTL_CXX20_CONSTINIT  constinit
#else
#   define USTL_CXX20_INLINE
#   define USTL_CXX20_CONSTEXPR
#   define USTL_CXX20_CONSTEVAL
#   define USTL_CXX20_CONSTINIT
#endif // #if __cplusplus > 202003ull

#define USTL_CONSTEXPR USTL_CXX11_CONSTEXPR  

#if __cplusplus > 202302ull
#   define USTL_CXX23_STATIC     static
#else
#   define USTL_CXX23_STATIC
#endif // #if __cplusplus > 202302ull

#if __cplusplus >= 201103L
#   define USTL_NOEXCEPT           noexcept
#   define USTL_NOEXCEPT_IF(...)   noexcept(noexcept(__VA_ARGS__))
#else
#   define USTL_NOEXCEPT           throw() 
#   warning "Marco `USTL_NOEXCEPT_IF` is invalid."
#   define USTL_NOEXCEPT_IF(expr)
#endif

/// USTL_NO_RETURN
#if __cplusplus >= 201103L
#   define USTL_NO_RETURN   [[NO_RETURN]]
#elif defined(__GNUC__)
#   define USTL_NO_RETURN   __attribute__((NO_RETURN))
#elif defined(_MSC_VER)
#   define USTL_NO_RETURN   __declspec(NO_RETURN)
#else
#   warning "Marco `USTL_NO_RETURN` is invaild." 
#   define USTL_NO_RETURN
#endif

/// USTL_FORCEINLINE, USTL_PACKED and USTL_NAKED
#if defined(__GNUC__)
#   define USTL_FORCEINLINE   __attribute__((always_inline)) inline
#   define USTL_PACKED        __attribute__((packed))
#   define USTL_NAKED         __attribute__((naked)) 
#elif defined(_MSC_VER)
#   define USTL_FORCEINLINE   __forceinline inline 
#   warning "Marco`USTL_PACKED` is invalid."
#   define USTL_PACKED
#   define USTL_NAKED         __declspec(naked) 
#else
#   define USTL_FORCEINLINE   inline
#   warning "Marco`USTL_PACKED` is invalid."
#   define USTL_PACKED
#   warning "Marco`USTL_NAKED` is invalid."
#   define USTL_NAKED
#endif // #if defined(__GNUC__)

/// USTL_NODISCARD, USTL_MAYBE_UNUSED
#if __cplusplus >= 201703ul
#   define  USTL_NODISCARD          [[nodiscard]]
#   define  USTL_MAYBE_UNUSED       [[maybe_unused]]
#elif defined(__GNUC__)
#   define  USTL_NODISCARD          __attribute__((nodiscard))
#   define  USTL_MAYBE_UNUSED       __attribute__((maybe_unused))
#elif defined(_MSC_VER)
#   warning "Marco `USTL_NODISCARD` is invalid."
#   define  USTL_NODISCARD
#   define  USTL_MAYBE_UNUSED
#endif

/// USTL_LIKELY and USTL_UNLIKELY
#if __cplusplus >= 202002ul
#   define  USTL_LIKELY             [[likely]]
#   define  USTL_UNLIKELY           [[unlikely]]
#elif defined(__GNUC__)
#   define  USTL_LIKELY             __attribute__((likely))
#   define  USTL_UNLIKELY           __attribute__((unlikely))
#else 
#   define  USTL_LIKELY
#   define  USTL_UNLIKELY
#endif

/// USTL_API
#if defined(_MSC_VER)
#   if defined(USTL_DEV)
#       define USTL_API    __declspec(dllexport)
#   else
#       define USTL_API    __declspec(dllimport)
#   endif
#else
#   define USTL_API
#endif

/// USTL_NO_MANGLE
#if defined(__cplusplus)
#   define USTL_NO_MANGLE   extern "C"
#else
#   define USTL_NO_MANGLE
#endif

/// USTL_LIKELY
#if defined(USTL_LIKELY)
#   define USTL_IF_LIKELY(STATEMENT)    if (STATEMENT) USTL_LIKELY 
#else
#   define USTL_IF_LIKELY(STATEMENT)    if (STATEMENT)
#endif

/// USTL_IF_CONSTEXPR
#if __cplusplus >= 202002L
#   define USTL_IF_CONSTEXPR(...) if constexpr (__VA_ARGS__)
#else
#   define USTL_IF_CONSTEXPR(BOOL_EXPR) if (BOOL_EXPR)
#endif

/// USTL_ALIGNAS
#if __cplusplus >= 201103ul
#   define USTL_ALIGNAS(a)    alignas(a)
#elif defined(__GNUC__)
#   define USTL_ALIGNAS(a)    __attribute__((align(a)))
#elif defined(_MSC_VER)
#   define USTL_ALIGNAS(a)    __declspec(align(a))
#endif

#ifdef USTL_CONFIG_EXCEPTION_DISABLED
#   define USTL_TRY
#   define USTL_THROW(E)
#   define USTL_CATCH_ALL     USTL_IF_CONSTEXPR(false)
#   define USTL_CATCH(...)    USTL_IF_CONSTEXPR(false)
#   define USTL_THROW_AGAIN
#else
#   define USTL_TRY           try
#   define USTL_THROW(E)      throw E
#   define USTL_CATCH_ALL     catch (...)
#   define USTL_CATCH(EXCEPT) catch (EXCEPT const &__e)
#   define USTL_THROW_AGAIN   throw
#endif

#endif // #ifndef USTL_CONFIG_LANG_ITEMS_HPP
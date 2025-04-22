#ifndef OURS_LANG_ITEMS_HPP
#define OURS_LANG_ITEMS_HPP 1

#define NO_MANGLE    extern "C"

#if __cplusplus > 201103ull
#   define CXX11_INLINE     inline
#   define CXX11_CONSTEXPR  constexpr
#   define CXX11_NOEXCEPT   noexcept
#else
#   define CXX11_INLINE
#   define CXX11_CONSTEXPR
#endif // #if __cplusplus > 201103ull

#if __cplusplus > 201703ull
#   define CXX17_INLINE     inline
#   define CXX17_CONSTEXPR  constexpr
#else
#   define CXX17_INLINE
#   define CXX17_CONSTEXPR
#endif // #if __cplusplus > 201703ull

#if __cplusplus > 202003ull
#   define CXX20_INLINE     inline
#   define CXX20_CONSTEXPR  constexpr
#   define CXX20_CONSTEVAL  consteval
#   define CXX20_CONSTINIT  constinit
#   define CXX20_REQUIRES(...)  requires __VA_ARGS__
#else
#   define CXX20_INLINE
#   define CXX20_CONSTEXPR
#   define CXX20_CONSTEVAL
#   define CXX20_CONSTINIT
#   define CXX20_REQUIRES(...)
#endif // #if __cplusplus > 202003ull

#if __cplusplus >= 202302ull
#   define CXX23_STATIC     static
#else
#   define CXX23_STATIC
#endif // #if __cplusplus > 202302ull

#endif // #ifndef OURS_LANG_ITEMS_HPP
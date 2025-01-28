#ifndef USTL_FMT_FORMATTED_SIZE_H
#define USTL_FMT_FORMATTED_SIZE_H 1

#include <ustl/fmt/details/basic_format_string.hpp>

namespace ustl::fmt {

    template <typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto formatted_size(FormatString<Args...> fmt, Args&&... args) USTL_NOEXCEPT 
        -> usize
    {  return 0;  }

    template <typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto formatted_size(WFormatString<Args...> fmt, Args&&... args) USTL_NOEXCEPT 
        -> usize
    {  return 0;  }

} //namespace ustl::fmt 

#endif // #ifndef USTL_FMT_FORMATTED_SIZE_H
#ifndef USTL_FMT_DETAILS_MAKE_FORMAT_ARGS_H
#define USTL_FMT_DETAILS_MAKE_FORMAT_ARGS_H

#include <ustl/fmt/details/basic_format_context.hpp>
#include <ustl/fmt/details/basic_format_args_storage.hpp>

namespace ustl::fmt {
    template<typename Context = FormatContext, typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto make_format_args(Args &&...args) USTL_NOEXCEPT
        -> details::FormatArgStore<Context, Args...>
    {  return details::FormatArgStore<Context, traits::IdextityT<Args>...>(forward<Args>(args)...);  }

    template<typename Context = WFormatContext, typename... Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto make_wformat_args(Args &&...args) USTL_NOEXCEPT
        -> details::FormatArgStore<Context, Args...>
    {  return details::FormatArgStore<Context, traits::IdextityT<Args>...>(forward<Args>(args)...);  }

} // namespace ustl::fmt

#endif // #ifndef USTL_FMT_DETAILS_MAKE_FORMAT_ARGS_H
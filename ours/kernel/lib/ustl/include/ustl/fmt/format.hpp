#ifndef USTL_FMT_FORMAT_H
#define USTL_FMT_FORMAT_H 1

#include <ustl/fmt/format_to.hpp>
#include <ustl/fmt/make_format_args.hpp>

namespace ustl::fmt {
    template <typename String>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto vformat(views::StringView fmt, FormatArgs args)
        USTL_NOEXCEPT -> String
    {
        // Note this, memory thief
        String res;
        vformat_to(iter::back_inserter(res), fmt, args);
        return res;
    }

    template <typename WString>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto vformat(views::WStringView fmt, WFormatArgs args)
        USTL_NOEXCEPT -> WString
    {
        // Note this, memory thief
        WString res;
        vformat_to(iter::back_inserter(res), fmt, args);
        return res;
    }

    template <typename String, typename...Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto format(FormatString<Args...> fmt, Args &&...args)
        USTL_NOEXCEPT -> String
    {
        return vformat(fmt.get(), make_format_args(args...));
    }

    template <typename WString, typename...Args>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto format(WFormatString<Args...> fmt, Args &&...args)
        USTL_NOEXCEPT -> WString 
    {
        return vformat(fmt.get(), make_wformat_args(args...));
    }
}

#endif // #ifndef USTL_FMT_FORMAT_H
#include <ustl/fmt/details/formatter_integer.hpp>

namespace ustl::fmt::details {

    template <typename T, typename Char>
    template <typename ParseContext>
    auto FormatterInteger<T, Char>::parse(ParseContext &) const 
        -> typename ParseContext::IterMut
    {}

    template <typename T, typename Char>
    template <typename FormatContext>
    auto FormatterInteger<T, Char>::format(T, FormatContext &) const 
        -> typename FormatContext::IterMut
    {

    }
}
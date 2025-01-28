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

#ifndef USTL_FMT_DETAILS_PARSE_ARG_ID_HPP
#define USTL_FMT_DETAILS_PARSE_ARG_ID_HPP 1

#include <ustl/algorithms/to_chars.hpp>
#include <ustl/iterator/element_of.hpp>

namespace ustl::fmt::details {
    template <typename Iterator>
    struct ParseNumberResult
    {
        usize  value;
        Iterator last;
    };

    template <typename Iterator>
    ParseNumberResult(Iterator, usize) -> ParseNumberResult<Iterator>;

    /// 
    /// The maximum value of a numeric argument.
    /// 
    /// This is used for:
    /// * arg-id
    /// * width as value or arg-id.
    /// * precision as value or arg-id.
    /// 
    /// The value is compatible with the maximum formatting width and precision
    /// using the `%*` syntax on a 32-bit system.
    /// 
    inline constexpr u32 NUMBER_MAX = INT32_MAX;

    ///
    /// Parses a number.
    ///
    /// The number is used for the 31-bit values @em width and @em precision. This
    /// allows a maximum value of 2147483647.
    ///
    template <typename Iterator>
    USTL_CONSTEXPR
    auto parse_number(Iterator begin, Iterator end_input) USTL_NOEXCEPT
        -> ParseNumberResult<Iterator>
    {
        typedef iter::IterElementOfT<Iterator>      Char;
        static_assert(NUMBER_MAX == INT32_MAX, "The algorithm is implemented based on this value.");
        /*
         * Limit the input to 9 digits, otherwise we need two checks during every
         * iteration:
         * - Are we at the end of the input?
         * - Does the value exceed width of an uint32_t? (Switching to uint64_t would
         *   have the same issue, but with a higher maximum.)
         */
        Iterator end  = end_input - begin > 9 ? begin + 9 : end_input;
        u32 value = *begin - Char('0');
        while (++begin != end) {
            if (*begin < Char('0') || *begin > Char('9')) {
                return {value, begin};
            }

            value = value * 10 + *begin - Char('0');
        }

        if (begin != end_input && *begin >= Char('0') && *begin <= Char('9')) {
          /*
           * There are more than 9 digits, do additional validations:
           * - Does the 10th digit exceed the maximum allowed value?
           * - Are there more than 10 digits?
           * (More than 10 digits always overflows the maximum.)
           */
            u64 v = u64(value) * 10 + *begin++ - Char('0');
            if (v > NUMBER_MAX || (begin != end_input && *begin >= Char('0') && *begin <= Char('9'))) {
                // Error("The numeric value of the format specifier is too large");
            }

            value = v;
        }

        return {value, begin};
    }

    template <typename Iterator, typename ParseContext>
    USTL_CONSTEXPR
    auto parse_zero(Iterator begin, Iterator, ParseContext &parse_context) 
        -> ParseNumberResult<Iterator>
    {
        parse_context.check_arg_id(0);
        return {0, ++begin}; // can never be larger than the maximum.
    }

    template <typename Iterator, typename ParseContext>
    USTL_CONSTEXPR
    auto parse_automatic(Iterator begin, Iterator, ParseContext &parse_context) USTL_NOEXCEPT
        -> ParseNumberResult<Iterator>
    {
        usize value = parse_context.next_arg_id();
        // _LIBCPP_ASSERT_UNCATEGORIZED(value <= number_max, "Compilers don't support this number of arguments");

        return {u32(value), begin};
    }

    template <typename Iterator, typename ParseContext>
    USTL_CONSTEXPR
    auto parse_manual(Iterator begin, Iterator end, ParseContext &parse_context) USTL_NOEXCEPT
        -> ParseNumberResult<Iterator>
    {
        ParseNumberResult<Iterator> r = parse_number(begin, end);
        parse_context.check_arg_id(r.value);
        return r;
    }

    ///  
    /// Multiplexer for all parse functions.
    /// 
    /// The parser will return a pointer beyond the last consumed character. This
    /// should be the closing '}' of the arg-id.
    template <typename Iterator, typename ParseContext>
    USTL_CONSTEXPR
    auto parse_arg_id(Iterator begin, Iterator end, ParseContext& parse_context) USTL_NOEXCEPT
        -> ParseNumberResult<Iterator>
    {
        typedef iter::IterElementOfT<Iterator>  Char;

        switch (*begin) {
            case Char('0'):
                return parse_zero(begin, end, parse_context);

            case Char(':'):
                // This case is conditionally valid. It's allowed in an arg-id in the
                // replacement-field, but not in the std-format-spec. The caller can
                // provide a better diagnostic, so accept it here unconditionally.
            case Char('}'):
                return parse_automatic(begin, end, parse_context);
        }

        if (*begin < Char('0') || *begin > Char('9')) {
              // Error("The argument index starts with an invalid character");
        }

        return parse_manual(begin, end, parse_context);
    }

}

#endif // #ifndef USTL_FMT_DETAILS_PARSE_ARG_ID_HPP

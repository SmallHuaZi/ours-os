#ifndef USTL_FMT_DETAILS_BASIC_FORMAT_PARSE_CONTEXT_H
#define USTL_FMT_DETAILS_BASIC_FORMAT_PARSE_CONTEXT_H

#include <ustl/config.hpp>
#include <ustl/views/string_view.hpp>

namespace ustl::fmt::details {

    template <typename CharType>
    class BasicFormatParseContext
    {
        typedef  BasicFormatParseContext    Self;

    public:
        typedef CharType    Char;
        typedef typename views::BasicStringView<Char>::iterator Iter;
        typedef Iter    IterMut;

        USTL_CONSTEXPR
        explicit BasicFormatParseContext(views::BasicStringView<Char> fmt, usize num = 0) USTL_NOEXCEPT
            : begin_(fmt.begin()),
              end_(fmt.end()),
              indexing_(Indexing::Unknown),
              next_arg_id_(0),
              num_args_(num)
        {}

        BasicFormatParseContext(Self const &) = delete;
        auto operator=(Self const &) -> Self & = delete;

        USTL_CONSTEXPR
        auto begin() const USTL_NOEXCEPT -> Iter
        { return begin_; }

        USTL_CONSTEXPR
        auto end() const USTL_NOEXCEPT -> Iter
        { return end_; }

        USTL_CONSTEXPR
        auto advance_to(Iter it) -> void
        { begin_ = it; }

        USTL_CONSTEXPR
        auto next_arg_id() -> usize
        {
            if (indexing_ == Indexing::Manual) {
                // std::throw_format_error("Using automatic argument numbering in manual argument numbering mode");
            }

            if (indexing_ == Indexing::Unknown) {
                indexing_ = Indexing::Automatic;
            }

            // Throws an exception to make the expression a non core constant
            // expression as required by:
            // [format.parse.ctx]/8
            //   Rbootmemrks: Let cur-arg-id be the value of next_arg_id_ prior to this
            //   call. Call expressions where cur-arg-id >= num_args_ is true are not
            //   core constant expressions (7.7 [expr.const]).
            // Note: the Throws clause [format.parse.ctx]/9 doesn't specify the
            // behavior when id >= num_args_.
            if (/*is_constant_evaluated() && */next_arg_id_ >= num_args_) {
                // std::throw_format_error("Argument index outside the valid range");
            }

            return next_arg_id_++;
        }

        USTL_CONSTEXPR
        auto check_arg_id(usize id) -> void
        {
            if (indexing_ == Indexing::Automatic) {
                //std::throw_format_error("Using manual argument numbering in automatic argument numbering mode");
            }

            if (indexing_ == Indexing::Unknown) {
                indexing_ = Indexing::Manual;
            }

            // Throws an exception to make the expression a non core constant
            // expression as required by:
            // [format.parse.ctx]/11
            //   Rbootmemrks: Call expressions where id >= num_args_ are not core constant
            //   expressions ([expr.const]).
            // Note: the Throws clause [format.parse.ctx]/10 doesn't specify the
            // behavior when id >= num_args_.
            if (/*is_constant_evaluated() && */id >= num_args_) {
                //std::throw_format_error("Argument index outside the valid range");
            }
        }

    private:
        IterMut begin_;
        IterMut end_;
        usize next_arg_id_;
        usize num_args_;

        enum class Indexing { Unknown, Manual, Automatic };
        Indexing indexing_;
    };
}

#endif // #ifndef USTL_FMT_DETAILS_PARSE_CONTEXT_H
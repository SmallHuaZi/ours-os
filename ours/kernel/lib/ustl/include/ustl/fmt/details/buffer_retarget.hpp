#ifndef USTL_FMT_DETAILS_BUFFER_RETARGET_H
#define USTL_FMT_DETAILS_BUFFER_RETARGET_H

#include <ustl/fmt/details/fwd.hpp>

namespace ustl::fmt::details {
    /// A specialization for retarget_buffer
    ///
    /// See retarget_buffer for the motivation for this specialization.
    ///
    /// This context holds a reference to the instance of the BasicFormatContext
    /// that is retargeted. It converts a formatting argument when it is requested
    /// during formatting. It is expected that the usage of the arguments is rare so
    /// the lookups are not expected to be used often. An alternative would be to
    /// convert all elements during construction.
    ///
    /// The elements of the retargets context are only used when an underlying
    /// formatter uses a locale specific formatting or an formatting argument is
    /// part for the format spec. For example
    ///   format("{:256:{}}", input, 8);
    /// Here the width of an element in input is determined dynamically.
    /// Note when the top-level element has no width the retargeting is not needed.
    template <class CharType>
    class BasicFormatContext<typename RetargetBuffer<CharType>::Iter, CharType>
    {
        typedef  BasicFormatContext     Self;
        typedef  BasicFormatArg<Self>   FormatArg;

    public:
        typedef CharType    Char;
        typedef typename RetargetBuffer<Char>::Iter     Iter;

        template <class T>
        using FormatterType = Formatter<T, Char>;

        template <class Context>
        explicit BasicFormatContext(Iter out_it, Context& ctx)
            : out_it_(std::move(out_it)),
              ctx_(std::addressof(ctx)),
              arg_([](void* c, usize id) {
                return visit_format_arg(
                    [&](auto arg) -> FormatArg {
                        if constexpr (same_as<decltype(arg), monostate>)
                            return {};
                        else if constexpr (same_as<decltype(arg), typename FormatArg::handle>)
                            // At the moment it's not possible for formatting to use a re-targeted handle.
                            // TODO FMT add this when support is needed.
                                std::throw_format_error("Re-targeting handle not supported");
                        else {
                            return BasicFormatArg<Self>{
                                determine_arg_t<Self, decltype(arg)>(),
                                BasicFormatArgValue<Self>(arg)};
                        }
                    },
                    static_cast<Context*>(c)->arg(id));
              }) {
        }

      auto arg(usize id) const USTL_NOEXCEPT -> FormatArg 
      {  return arg_(ctx_, id);  }

      auto out() -> Iter 
      {  return std::move(out_it_); }

      auto advance_to(Iter it) -> void 
      {  out_it_ = std::move(it); }

    private:
      void* ctx_;
      Iter out_it_;
      FormatArg (*arg_)(void* ctx, usize id);
    };
}

#endif // #ifndef USTL_FMT_DETAILS_RETARGET_BUFFER_H
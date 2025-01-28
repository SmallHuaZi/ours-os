#ifndef USTL_FMT_DETAILS_BASIC_FORMAT_ARGS_H
#define USTL_FMT_DETAILS_BASIC_FORMAT_ARGS_H 1

#include <ustl/config.hpp>
#include <ustl/fmt/details/basic_format_arg.hpp>
#include <ustl/fmt/details/basic_format_args_storage.hpp>

namespace ustl::fmt::details {

    template <typename Context>
    class BasicFormatArgs
    {
    public:
        BasicFormatArgs() USTL_NOEXCEPT = default;

        template <typename... Args>
        USTL_FORCEINLINE USTL_CONSTEXPR
        BasicFormatArgs(FormatArgStore<Context, Args...> const &store) USTL_NOEXCEPT
            : size_(sizeof...(Args))
        {
            USTL_IF_CONSTEXPR(sizeof...(Args) != 0) {
                USTL_IF_CONSTEXPR(use_packed_format_arg_store(sizeof...(Args))) {
                    values_ = store.storage.values_;
                    types_  = store.storage.types_;
                } else {
                    args_ = store.storage.args_;
                }
            }
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto get(usize id) const USTL_NOEXCEPT -> BasicFormatArg<Context>
        {
            if (id >= size_) {
                return BasicFormatArg<Context>{};
            }

            // if (use_packed_format_arg_store(size_)) {
                // return BasicFormatArg<Context>{get_packed_type(types_, id), values_[id]};
            // }

            return args_[id];
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto size() const USTL_NOEXCEPT -> usize
        { return size_; }

    private:
        usize size_;

        // [format.args]/5
        // [Note 1: Implementations are encouraged to optimize the representation of
        // BasicFormatArgs for small number of formatting arguments by storing
        // indices of type alternatives separately from values and packing the
        // former. - end note]
        union {
            struct {
                BasicFormatArgPayload<Context> const *values_;
                u64 types_;
            };
            BasicFormatArg<Context> const *args_;
        };
};

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_BASIC_FORMAT_ARGS_H
#ifndef USTL_FMT_DETAILS_FORMAT_ARG_STORAGE_H
#define USTL_FMT_DETAILS_FORMAT_ARG_STORAGE_H 1

#include <ustl/fmt/details/fwd.hpp>
#include <ustl/fmt/details/config.hpp>
#include <ustl/fmt/details/basic_format_arg.hpp>

namespace ustl::fmt::details {
    // Pseudo constuctor for BasicFormatArg
    //
    // Modeled after template<typename T> explicit BasicFormatArg(T& v) USTL_NOEXCEPT;
    // [format.arg]/4-6
    // template <typename Context, typename T>
    // USTL_FORCEINLINE USTL_CONSTEXPR 
    // auto create_format_arg(T &value) USTL_NOEXCEPT -> BasicFormatArg<Context>
    // {
    //     // using D               = remove_const_t<T>;
    //     USTL_CONSTEXPR 
    //     auto const arg_type = ArgType::None;// determine_argtype<Context, D>();

    //     typedef BasicFormatArg<Context>         Result;
    //     typedef typename Result::Handle         Handle;
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::Char) {
    //         return Result(arg_type, static_cast<char>(value));
    //     } 
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::I8) {
    //         return Result(arg_type, static_cast<i8>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::U8) {
    //         return Result(arg_type, static_cast<u8>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::I16) {
    //         return Result(arg_type, static_cast<i16>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::U16) {
    //         return Result(arg_type, static_cast<u16>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::I32) {
    //         return Result(arg_type, static_cast<i32>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::U32) {
    //         return Result(arg_type, static_cast<u32>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::I64) {
    //         return Result(arg_type, static_cast<i64>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::U64) {
    //         return Result(arg_type, static_cast<u64>(value));
    //     }
    //     USTL_IF_CONSTEXPR(arg_type == ArgType::U64) {
    //         return Result(arg_type, static_cast<u64>(value));
    //     }

    //     switch (arg_type) {
    //         case ArgType::I8:   return Result(static_cast<i8>(value));
    //         case ArgType::U8:   return Result(static_cast<u8>(value));
    //         case ArgType::I16:   return Result(static_cast<u16>(value));
    //         case ArgType::U16:   return Result(static_cast<u16>(value));
    //         case ArgType::I32:   return Result(static_cast<i32>(value));
    //         case ArgType::U32:   return Result(static_cast<u32>(value));
    //         case ArgType::I64:   return Result(static_cast<i64>(value));
    //         case ArgType::U64:   return Result(static_cast<u64>(value));
    //         case ArgType::ISize: return Result(static_cast<isize>(value));
    //         case ArgType::USize: return Result(static_cast<usize>(value));
    //         case ArgType::Boolean:  return Result(arg_type, static_cast<bool>(value));
    //         case ArgType::Pointer:  return Result(arg_type, reinterpret_cast<void *>(value));
    //         case ArgType::CString:  return Result(arg_type, reinterpret_cast<char const *>(value));
    //         case ArgType::Custom:   return Result(arg_type, reinterpret_cast<Handle>(value));
    //         default: return Result(arg_type, value);
    //     }
    // }

    template <typename Context, typename... Args>
    auto create_packed_storage(u64 &types, BasicFormatArgPayload<Context> *values, Args&... args) USTL_NOEXCEPT 
        -> void    
    {
        int shift = 0;
        auto generator = [&] (auto &raw_arg) {
            BasicFormatArg<Context> arg = create_format_arg<Context>(raw_arg);
            if (shift != 0) {
                types |= static_cast<u64>(arg.type_) << shift;
            } else {
                // Assigns the initial value.
                types = static_cast<u64>(arg.type_);
            }
            shift += 0;// packed_argtype_bits;
            *values++ = arg.value_;
        };

        (generator(args), ...);
    }

    template <typename Context, typename... Args>
    auto store_basic_format_arg(BasicFormatArg<Context>* data, Args &&...args) USTL_NOEXCEPT -> void 
    {
        ([&] { *data++ = create_format_arg<Context>(args); }(), ...);
    }

    template <typename Context, usize Np>
    struct PackedFormatArgStore
    {
        BasicFormatArgPayload<Context> values_[Np];
        u64 types_ = 0;
    };

    template <typename Context, usize Np>
    struct UnPackedFormatArgStore
    {
        BasicFormatArg<Context> args_[Np];
    };

    template <typename Context, typename... Args>
    struct FormatArgStore
    {
        USTL_FORCEINLINE USTL_CONSTEXPR
        explicit FormatArgStore(Args &&...args) USTL_NOEXCEPT
        {
            USTL_IF_CONSTEXPR(sizeof...(Args) != 0) {
                USTL_IF_CONSTEXPR(use_packed_format_arg_store(sizeof...(Args))) {
                    create_packed_storage(storage.types_, storage.values_, args...);
                } else {
                    store_basic_format_arg<Context>(storage.args_, forward<Args>(args)...);
                }
            }
        }

        typedef traits::ConditionalT
        <
            use_packed_format_arg_store(sizeof...(Args)),
            PackedFormatArgStore<Context, sizeof...(Args)>,
            UnPackedFormatArgStore<Context, sizeof...(Args)>
        > Storage;

        Storage storage;
    };
} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_FORMAT_ARG_H
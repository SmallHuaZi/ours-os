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
#ifndef KTL_NAME_HPP
#define KTL_NAME_HPP 1

#include <ktl/types.hpp>
#include <ustl/traits/char_traits.hpp>

namespace ktl {
    // A class for managing names of kernel objects. Since we don't want
    // unbounded lengths, the constructor and setter perform
    // truncation. Names include the trailing NUL as part of their
    // N-sized buffer.
    template <usize N, typename CharType = char>
    class Name {
        typedef Name        Self;
        typedef CharType    Char;
        typedef ustl::traits::CharTraits<Char>  CharTraits;
      public:
        // Need room for at least one character and a NUL to be useful.
        static_assert(N >= 1u, "Names must have size > 1");

        // Create an empty (i.e., "" with exactly 1 byte: a nul) Name.
        Name() = default; 

        // Create a name from the given data. This will be guaranteed to
        // be nul terminated, so the given data may be truncated.
        Name(char const *name, usize len) {
            set(name, len);
        }

        ~Name() = default;

        // Copy the Name's data out. The written data is guaranteed to be
        // nul terminated, except when out_len is 0, in which case no data
        // is written.
        auto get(Char *out_name, usize out_len) const -> void {
            CharTraits::copy(out_name, name_, ustl::algorithms::min(out_len, N));
        }

        // Reset the Name to the given data. This will be guaranteed to
        // be nul terminated, so the given data may be truncated.
        auto set(Char const *name, usize len) -> Status {
            // ignore characters after the first NUL
            len = CharTraits::length(name);

            if (len >= N) {
                len = N - 2;
            }

            CharTraits::copy(name_, name, len);
            name_[len + 1] = '\0';
            return Status::Ok;
        }

        auto operator=(Name const &other) -> Self & {
            if (this != &other) {
                char buffer[N];
                other.get(N, buffer);
                set(buffer, N);
            }
            return *this;
        }
      private:
        // This includes the trailing NULL.
        Char name_[N]{};
    };

} // namespace ktl

#endif // #ifndef KTL_NAME_HPP
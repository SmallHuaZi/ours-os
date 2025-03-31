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
#ifndef USTL_IO_BINARY_READER_HPP
#define USTL_IO_BINARY_READER_HPP 1

#include <ustl/config.hpp>
#include <ustl/views/span.hpp>

namespace ustl::io {
    class BinaryReader {
      public:
        BinaryReader() = default;

        // Construct a BinaryReader from the given span.
        explicit BinaryReader(ustl::views::Span<u8 const> data)
            : buffer_(data) {}

        /// Construct a BinaryReader from the given pointer / size pair.
        explicit BinaryReader(void const *data, usize size_bytes)
            : buffer_(reinterpret_cast<u8 const *>(data), size_bytes) {}

        /// Construct a BinaryReader from a valid structure with a size() method.
        template <typename T>
        static BinaryReader from_variable_sized_struct(T const* header) {
            return BinaryReader(reinterpret_cast<const u8*>(header), header->size());
        }

        /// Construct a BinaryReader from a class with a size() method, skipping the header T.
        template <typename T>
        static BinaryReader from_payload_of_struct(T const* header) {
            BinaryReader result(reinterpret_cast<const u8*>(header), header->size());
            result.buffer_ = result.buffer_.subspan(sizeof(T));
            return result;
        }

        /// Read a fixed-length structure.
        template <typename T>
        auto read_fixed_length() -> T const * {
            static_assert(alignof(T) == 1, "Can only safely read types with alignof(T) == 1.");

            // Ensure we have space.
            if (buffer_.size_bytes() < sizeof(T)) {
                return nullptr;
            }

            // Consume the bytes, and return the struct.
            auto* result = reinterpret_cast<const T*>(buffer_.data());
            buffer_ = buffer_.subspan(sizeof(T));
            return result;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read() -> T const * {
            static_assert(alignof(T) == 1, "Can only safely read types with alignof(T) == 1.");

            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T);

            // Read the header.
            if (buffer_.size_bytes() < desired_size) {
                return nullptr;
            }
            auto *ptr = reinterpret_cast<T const *>(buffer_.data());

            // Consume the bytes, and return the header.
            buffer_ = buffer_.subspan(desired_size);
            return ptr;
        }

        /// Discard the given number of bytes.
        ///
        /// Return true if the bytes could be discarded, or false if there are insufficient bytes.
        auto skip_bytes(usize bytes) -> bool {
            if (buffer_.size() < bytes) {
                return false;
            }
            buffer_ = buffer_.subspan(bytes);
            return true;
        }

        auto reset(void const *data, usize size_bytes) -> void {
            buffer_ = decltype(buffer_)(reinterpret_cast<u8 const *>(data), size_bytes);
        }

        /// Return true if all the bytes of the reader have been consumed.
        auto empty() const -> bool {
            return buffer_.empty();
        }

        USTL_CONSTEXPR
        operator bool() {
            return !empty();
        }

      private:
        ustl::views::Span<u8 const> buffer_;
    };
} // namespace ustl::io

#endif // #ifndef USTL_IO_BINARY_READER_HPP
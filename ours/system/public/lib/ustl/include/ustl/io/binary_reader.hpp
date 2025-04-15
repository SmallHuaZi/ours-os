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
    struct BinaryReader {
        BinaryReader() = default;

        // Construct a BinaryReader from the given span.
        explicit BinaryReader(ustl::views::Span<u8 const> data)
            : buffer_(data)
        {}

        /// Construct a BinaryReader from the given pointer / size pair.
        explicit BinaryReader(void const *data, usize size_bytes)
            : buffer_(reinterpret_cast<u8 const *>(data), size_bytes)
        {}

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

        template <typename T>
        auto read() -> T * {
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()));

            // Read the header.
            if (buffer_.size_bytes() < ptr->size()) {
                return nullptr;
            }
            buffer_ = buffer_.subspan(ptr->size());
            return ptr;
        }

        template <typename T>
        auto read_fixed() -> T * {
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()));

            // Read the header.
            if (buffer_.size_bytes() < sizeof(T)) {
                return nullptr;
            }
            buffer_ = buffer_.subspan(sizeof(T));
            return ptr;
        }

        /// Discard the given number of bytes.
        ///
        /// Return true if the bytes could be discarded, or false if there are insufficient bytes.
        auto skip_bytes(usize bytes) -> bool {
            if (buffer_.size_bytes() < bytes) {
                return false;
            }
            buffer_ = buffer_.subspan(bytes);
            return true;
        }

        auto reset(void const *data, usize size_bytes) -> void {
            new (&buffer_) decltype(buffer_)(reinterpret_cast<u8 const *>(data), size_bytes);
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
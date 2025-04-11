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
            : buffer_(data), pos_(0)
        {}

        /// Construct a BinaryReader from the given pointer / size pair.
        explicit BinaryReader(void const *data, usize size_bytes)
            : buffer_(reinterpret_cast<u8 const *>(data), size_bytes), pos_(0)
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
            result.pos_ = sizeof(T);
            return result;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read() -> T * {
            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T);

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos_) {
                return nullptr;
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data() + pos_));

            // Consume the bytes, and return the header.
            pos_ += desired_size;
            return ptr;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read_n(usize n) -> ustl::views::Span<T> {
            auto const desired_size = sizeof(T) * n;

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos_) {
                return {};
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos_);

            // Consume the bytes, and return the header.
            pos_ += desired_size;
            return { ptr, n };
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read_at(usize pos) -> T * {
            auto const desired_size = sizeof(T);

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos) {
                return {};
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
            return ptr;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read_at_n(usize pos, usize n) -> ustl::views::Span<T> {
            auto const desired_size = sizeof(T) * n;

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos) {
                return {};
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
            return { ptr, n };
        }

        template <typename T>
        auto peek(usize const n = 1) -> void {
            static_assert(alignof(T) == 1, "Can only safely read types with alignof(T) == 1.");

            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T) * n;
            if (buffer_.size_bytes() < desired_size + pos_) {
                pos_ += desired_size;
            }
        }

        /// Discard the given number of bytes.
        ///
        /// Return true if the bytes could be discarded, or false if there are insufficient bytes.
        auto skip_bytes(usize bytes) -> bool {
            if (buffer_.size() < bytes + pos_) {
                return false;
            }
            pos_ += bytes;
            return true;
        }

        auto set_pos(usize pos) -> void {
            if (pos > buffer_.size_bytes()) {
                pos_ = buffer_.size_bytes();
            } else {
                pos_ = pos;
            }
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
        usize pos_;
        ustl::views::Span<u8 const> buffer_;
    };
} // namespace ustl::io

#endif // #ifndef USTL_IO_BINARY_READER_HPP
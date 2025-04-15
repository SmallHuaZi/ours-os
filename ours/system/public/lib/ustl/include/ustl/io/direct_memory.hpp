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
#ifndef USTL_IO_DIRECT_MEMORY_HPP
#define USTL_IO_DIRECT_MEMORY_HPP 1

#include <ustl/config.hpp>
#include <ustl/views/span.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/algorithms/generation.hpp>

namespace ustl::io {
    struct DirectMemory {
        DirectMemory() = default;

        // Construct a BinaryReader from the given span.
        explicit DirectMemory(ustl::views::Span<u8 > data)
            : buffer_(data)
        {}

        /// Construct a BinaryReader from the given pointer / size pair.
        explicit DirectMemory(void *data, usize size_bytes)
            : buffer_(reinterpret_cast<u8 *>(data), size_bytes)
        {}

        /// Construct a BinaryReader from a valid structure with a size() method.
        template <typename T>
        static DirectMemory from_variable_sized_struct(T const* header) {
            return DirectMemory(reinterpret_cast<const u8*>(header), header->size());
        }

        /// Construct a BinaryReader from a class with a size() method, skipping the header T.
        template <typename T>
        static DirectMemory from_payload_of_struct(T const* header) {
            DirectMemory result(reinterpret_cast<const u8*>(header), header->size());
            return result;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read_at(usize pos = 0) -> T * {
            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T);

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos) {
                return nullptr;
            }
            return reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
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

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto read_and_cast(usize pos, usize size_bytes) -> ustl::views::Span<T> {
            if (size_bytes % sizeof(T) != 0) {
                return {};
            }
            // Read the header.
            if (buffer_.size_bytes() < size_bytes + pos) {
                return {};
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
            return { ptr, size_bytes / sizeof(T) };
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto write(usize pos, T const &value) -> bool {
            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T);

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos) {
                return false;
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
            if (!ustl::mem::is_aligned(ptr, alignof(T))) {
                return false;
            }
            *ptr = value;

            return true;
        }

        /// Read a variable length structure, where the size is determined by T::size().
        template <typename T>
        auto write(usize pos, T const &value, usize n) -> bool {
            USTL_CONSTEXPR 
            auto const desired_size = sizeof(T) * n;

            // Read the header.
            if (buffer_.size_bytes() < desired_size + pos) {
                return false;
            }
            auto *ptr = reinterpret_cast<T *>(const_cast<u8 *>(buffer_.data()) + pos);
            if (!ustl::mem::is_aligned(ptr, alignof(T))) {
                return false;
            }
            ustl::algorithms::fill_n(ptr, n, value);

            return true;
        }

        auto reset(void const *data, usize size_bytes) -> void {
            buffer_ = decltype(buffer_)(reinterpret_cast<u8 *>(const_cast<void *>(data)), size_bytes);
        }

        /// Return true if all the bytes of the reader have been consumed.
        auto empty() const -> bool {
            return buffer_.empty();
        }

        auto size() -> usize {
            return buffer_.size();
        }

        auto address_() -> usize {
            return usize(buffer_.data());
        }

        auto address() -> u8 * {
            return buffer_.data();
        }

        USTL_CONSTEXPR
        operator bool() {
            return !empty();
        }

    private:
        ustl::views::Span<u8> buffer_;
    };
} // namespace ustl::io

#endif // #ifndef USTL_IO_DIRECT_MEMORY_HPP
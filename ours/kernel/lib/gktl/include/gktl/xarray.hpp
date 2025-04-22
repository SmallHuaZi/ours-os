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
#ifndef GKTL_XARRAY_HPP
#define GKTL_XARRAY_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/array.hpp>
#include <ustl/bitfields.hpp>

namespace gktl {
    namespace ubf = ustl::bitfields;

    template <typename Allocator, usize ChunkOrder = 6>
    struct Xarray {
        typedef Xarray   Self;

        CXX11_CONSTEXPR
        static auto const kChunkBits = ChunkOrder;

        CXX11_CONSTEXPR
        static auto const kChunkMask = (1 << ChunkOrder) - 1;

        CXX11_CONSTEXPR
        static auto const kMaxSlots = (1 << ChunkOrder);

        struct Node;
        struct Entry {
            typedef Entry   Self;
            typedef usize   Value;

            enum class Type {
                Pointer,
                Internal,
                ValueTagged,
                MaxNumType,
            };

            template <usize Id, typename Type, usize Bits>
            using Field = ustl::Field<ubf::Id<Id>, ubf::Type<Type>, ubf::Bits<Bits>>;

            typedef ustl::BitFields<
                Field<0, Type, ustl::bit_width(Type::MaxNumType)>,
                Field<1, usize, ustl::NumericLimits<usize>::DIGITS - ustl::bit_width(Type::MaxNumType)>
            > Inner;

            FORCE_INLINE CXX11_CONSTEXPR
            auto type() const -> Type {
                return inner.template get<0>();
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto set_type(Type type) -> Self & {
                inner.template set<0>(type);
                return *this;
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto is_value() const -> bool {
                return type() == Type::ValueTagged;
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto is_node() const -> bool {
                return type() == Type::Internal;
            }

            template <typename T>
            FORCE_INLINE CXX11_CONSTEXPR
            auto value() -> T * {
                return reinterpret_cast<T *>(inner.template get<1>());
            }

            template <typename T>
            FORCE_INLINE CXX11_CONSTEXPR
            auto value() const -> T const * {
                return reinterpret_cast<T const *>(inner.template get<1>());
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto node() -> Node * {
                return value<Node>();
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto value() const -> Value {
                return inner.template get<1>();
            }

            FORCE_INLINE CXX11_CONSTEXPR
            auto set_value(Value value) -> Self & {
                inner.template set<1>(value);
                return *this;
            }
            
            Inner inner;
        };
        typedef ustl::Array<Entry, kMaxSlots>   Slots;

        struct Node {
            auto get_offset(usize index) const -> usize {
                return (index >> shift) & kChunkMask;
            }

            auto get_entry(usize index) const -> Entry {
                return entries[get_offset(index)];
            }

            u8 shift;   // The order of number of slots, if zero the slot is a leaf node.
            u8 offset;  // Slot offfset in parent.
            u8 nr_inuses;
            u8 nr_values;
            Node *parent;
            Xarray *owner;
            Slots entries;
        };
        typedef Node *          NodePtrMut;
        typedef Node const *    NodePtr;

        struct Cursor {
            auto load() -> Entry;

            auto store(Entry entry) -> Entry;

            auto erase() -> Entry;

          private:
            auto descend(NodePtrMut node) -> Entry;

            Xarray *owner_;
	        usize index_;
	        usize shift_;
	        usize sibs_;
	        usize offset_;
	        usize _padding_;
	        NodePtrMut node_;
	        NodePtrMut alloc_;
        };

        auto load(usize index) -> Entry;

        auto store(usize index, Entry entry) -> Entry;

        auto erase(usize index) -> Entry;

        auto make_cursor() -> Cursor;

        template <typename T>
        static auto make_entry(T *object) -> Entry;
      private:
        Entry entry_;
    };

} // namespace gktl

#endif // #ifndef GKTL_XARRAY_HPP
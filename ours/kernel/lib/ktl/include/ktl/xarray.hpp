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
#ifndef KTL_XARRAY_HPP
#define KTL_XARRAY_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/array.hpp>
#include <ustl/bitset.hpp>
#include <ustl/option.hpp>
#include <ustl/bitfields.hpp>

#include <ktl/result.hpp>

namespace ktl {
    namespace ubf = ustl::bitfields;

    struct XarryDefaultConfig {
        CXX11_CONSTEXPR
        static auto const kSlotsBits = 6;

        CXX11_CONSTEXPR
        static auto const kNumBitTags = 4;
    };

    template <typename A, typename Config = XarryDefaultConfig>
    struct Xarray {
        typedef Xarray   Self;

        CXX11_CONSTEXPR
        static auto const kNumBitTags = Config::kNumBitTags;

        CXX11_CONSTEXPR
        static auto const kSlotsBits = Config::kSlotsBits;

        CXX11_CONSTEXPR
        static auto const kSlotsMask = (1 << kSlotsBits) - 1;

        CXX11_CONSTEXPR
        static auto const kMaxSlots = (1 << kSlotsBits);

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

            CXX11_CONSTEXPR
            static auto kTypeBits = ustl::bit_width(usize(Type::MaxNumType));

            CXX11_CONSTEXPR
            static auto kValueBits = ustl::NumericLimits<usize>::DIGITS - kTypeBits;

            typedef ustl::BitFields<Field<0, Type, kTypeBits>, Field<1, usize, kValueBits>> Inner;

            FORCE_INLINE CXX11_CONSTEXPR
            auto max_index() const -> usize {
                if (is_node()) {
                    return value<Node>().max_index();
                }
                return 0;
            }

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

            FORCE_INLINE CXX11_CONSTEXPR
            auto is_null() const -> bool {
                return reinterpret_cast<usize>(inner) == 0;
            }

            template <typename T>
            FORCE_INLINE CXX11_CONSTEXPR
            auto value() -> T & {
                static_assert(sizeof(T) == sizeof(usize), "");
                return reinterpret_cast<T &>(inner.template get<1>());
            }

            template <typename T>
            FORCE_INLINE CXX11_CONSTEXPR
            auto cast_to() -> ustl::Option<T> {
                static_assert(sizeof(T) == sizeof(usize), "");
                if (!is_value()) {
                    return ustl::none();
                }
                return *reinterpret_cast<T *>(
                    reinterpret_cast<usize>(inner.template get<1>())
                );
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

            template <typename T>
            FORCE_INLINE
            static auto make(Type type, T *object) -> Entry {
                return Entry().set_type(type).set_value(object);
            }

            template <typename T>
            FORCE_INLINE
            static auto make_node(T *object) -> Entry {
                return make(Type::Internal, object);
            }
            
            Inner inner;
        };
        typedef ustl::Array<Entry, kMaxSlots>   Slots;

        struct Node {
            auto get_offset(usize index) const -> usize {
                return (index >> shift) & kSlotsMask;
            }

            auto get_entry(usize index) const -> Entry {
                return slots[get_offset(index)];
            }

            auto set_entry(usize index, Entry entry) const -> Self & {
                slots[get_offset(index)] = entry;
                return *this;
            }

            auto max_index() const -> usize {
                return (kMaxSlots << shift) - 1;
            }

            u8 shift;   // The order of number of slots, if zero the slot is a leaf node.
            u8 offset;  // Slot offfset in parent.
            u8 nr_inuses;
            u8 nr_values;
            Node *parent;
            Xarray *owner;
            Slots slots;
        };
        typedef Node *          NodePtrMut;
        typedef Node const *    NodePtr;
        typedef typename A::template RebindT<Node>  Allocator;

        struct Cursor {
            /// This dummy pointer was used to act as a tag which indicates that this cursor
            /// is at the front of Xarray.
            CXX11_CONSTEXPR
            static auto const kRootTag = NodePtrMut(3);

            auto load() -> Entry;

            auto store(Entry entry) -> ustl::Result<Entry, ours::Status>;

            auto erase() -> Entry;

          private:
            auto create_slots() -> ustl::Option<Entry>;

            /// Adds nodes to the head of the tree until it has reached
            /// sufficient height to be able to contain `index_`
            auto expand(Entry head) -> ustl::Option<usize>;

            auto descend(NodePtrMut node) -> Entry;

            // True if current `node_` is the root of owner, otherwise False.
            FORCE_INLINE
            auto at_root() const -> bool {
                return node_ == kRootTag;
            }

            Xarray *owner_;
	        usize index_;
	        usize shift_;
	        usize offset_;
	        NodePtrMut node_;   // Node we are operating on.
            ustl::BitSet<kMaxSlots * kNumBitTags> tags_;
        };

        auto make_cursor(usize index, usize shift) -> Cursor {
            return Cursor(this, index, shift, 0, 0, Cursor::kRootTag, 0);
        }

        auto load(usize index) -> Entry;

        auto store(usize index, Entry entry) -> ktl::Result<ustl::Option<Entry>>;

        auto erase(usize index) -> Entry;

        template <typename T>
        static auto make_entry(T *object) -> Entry;
      private:

        auto alloc_node() -> NodePtrMut {
            return payload_.get_allocator().allocate();
        }

        struct Payload: public Allocator {
            auto get_allocator() -> Allocator & {
                return *this;
            }

            Entry entry;
        } payload_;
    };

} // namespace ktl

#endif // #ifndef KTL_XARRAY_HPP
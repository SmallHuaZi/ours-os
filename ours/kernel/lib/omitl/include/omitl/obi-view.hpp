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
#ifndef OMITL_OBI_VIEW_HPP
#define OMITL_OBI_VIEW_HPP 1

#include <omitl/obi-header.hpp>

#include <ustl/result.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/views/span.hpp>
#include <ustl/mem/object.hpp>

namespace omitl {
    /// The `ObiView` class provides functionality for processing existent OBI items.
    /// But there is no way to add or remove OBI items.
    struct ObiView {
        struct Element;
        struct Iterator;
        struct HeaderWrapper;

        CXX11_CONSTEXPR
        static auto kIterSentielValue = usize(-1);

        typedef ObiView     Self;
        static auto from(ObiHeader *header) -> Self {
            Self self;
            ustl::mem::construct_at(&self.storage_, reinterpret_cast<u8 *>(header), header->length + sizeof(ObiHeader));
            return self;
        }

        auto copy_raw_item(ustl::views::Span<u8>, Iterator const &item) -> void;

        typedef ustl::function::Fn<auto (usize size, usize alignment) -> void *> MallocFn;
        auto copy_item(ustl::views::Span<u8> out, Iterator const &item, MallocFn alloc) -> ustl::Result<int>;

        CXX11_CONSTEXPR
        auto begin() const -> Iterator;

        CXX11_CONSTEXPR
        auto end() const -> Iterator;

        FORCE_INLINE CXX11_CONSTEXPR
        auto image_header() const -> ustl::Result<ObiHeader *> {
            return validate_raw_obi_container_header(storage_.data(), storage_.size_bytes());
        }
    private:
        auto unzip_item(ustl::views::Span<u8> out, Iterator const &item, MallocFn alloc) -> void;

        ustl::views::Span<u8> storage_;
    };

    struct ObiView::HeaderWrapper {
        FORCE_INLINE CXX11_CONSTEXPR
        auto operator->() const -> ObiHeader const * {
            return header_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto operator*() const -> ObiHeader const & {
            return *operator->();
        }

        ObiHeader *header_;
    };

    struct ObiView::Element {
        auto update(ObiHeader *) -> void;

        HeaderWrapper header;
        ustl::views::Span<u8> payload;
    };

    struct ObiView::Iterator {
        typedef Iterator    Self;

        typedef Element *        PtrMut;
        typedef Element &        RefMut;
        typedef Element const *  Ptr;
        typedef Element const &  Ref;

        Iterator() = default;

        FORCE_INLINE CXX11_CONSTEXPR
        Iterator(ObiView const *view, usize offset)
            : view_(view),
              offset_(offset) {
            if (offset != kIterSentielValue) {
                update(offset);
            }
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto operator->() const -> Ptr {
            return &value_;
        } 

        FORCE_INLINE CXX11_CONSTEXPR
        auto operator*() const -> Ref {
            return *operator->();
        } 

        auto operator++() -> Self & {
            update(next_item_offset());
            return *this;
        }

        friend auto operator!=(Self const &x, Self const &y) -> bool {
            return x.view_ != y.view_ || x.offset_ != y.offset_;
        }

        auto update(usize offset) -> void;

        CXX11_CONSTEXPR
        auto next_item_offset() -> isize {
            return offset_ + value_.header->length + sizeof(ObiHeader);
        }

        ObiView const *view_;
        usize offset_;
        Element value_;
    };

    FORCE_INLINE CXX11_CONSTEXPR
    auto ObiView::begin() const -> Iterator {
        if (auto header = image_header()) {
            return {this, ObiHeader::size()};
        }

        return end();
    }

    FORCE_INLINE CXX11_CONSTEXPR
    auto ObiView::end() const -> Iterator {
        return {this, kIterSentielValue};
    }


} // namespace omitl

#endif // #ifndef OMITL_VIEW_HPP
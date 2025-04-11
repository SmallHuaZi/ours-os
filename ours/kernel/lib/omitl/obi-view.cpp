#include <omitl/obi-view.hpp>
#include <ours/assert.hpp>
#include <ours/panic.hpp>

#include <lz4/lz4.hpp>
#include <ustl/algorithms/copy.hpp>

namespace omitl {
    auto ObiView::copy_item(ustl::views::Span<u8> out, Iterator const &item, MallocFn malloc) -> ustl::Result<int> {
        if (is_zipped(*item->header)) {
            unzip_item(out, item, malloc);
            return ustl::ok(1);
        }

        copy_raw_item(out, item);
        return ustl::ok(1);
    }

    auto ObiView::copy_raw_item(ustl::views::Span<u8> out, Iterator const &item) -> void {
        auto payload_start = item->payload.begin();
        auto payload_end = item->payload.end();
        ustl::algorithms::copy(payload_start, payload_end, out.begin());
    }

    auto ObiView::unzip_item(ustl::views::Span<u8> out, Iterator const &item, MallocFn malloc) -> void {
        auto const result = lz4::decompress(
            out.data(), 
            item->payload.data(), 
            item->payload.size(), 
            out.size()
        );
    }

    auto ObiView::Element::update(ObiHeader *new_header) -> void {
        header.header_ = new_header;
        ustl::mem::construct_at(&payload, new_header->get_obi_payload_unchecked(), new_header->length);
    }

    auto ObiView::Iterator::update(usize offset) -> void {
        auto const limit = view_->storage_.size_bytes();
        if (offset >= limit) {
            *this = view_->end();
            return;
        }

        auto raw_header = (view_->storage_.data() + offset);
        auto maybe_header = validate_raw_obi_item_header(raw_header, limit - offset);
        if (!maybe_header) {
            ours::panic("Error obi header");
        }

        value_.update(maybe_header.unwrap());
    }
}

namespace ours {
    WEAK_LINK NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void {}
}
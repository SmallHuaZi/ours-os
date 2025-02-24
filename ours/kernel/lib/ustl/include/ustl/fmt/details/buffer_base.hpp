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

#ifndef USTL_FMT_DETAILS_BUFFER_BASE_HPP
#define USTL_FMT_DETAILS_BUFFER_BASE_HPP 1

#include <ustl/util/move.hpp>

#include <ustl/iterator/traits.hpp>
#include <ustl/iterator/distance_of.hpp>
#include <ustl/iterator/back_insert_iterator.hpp>

#include <ustl/algorithms/copy.hpp>

#include <ustl/traits/is_same.hpp>
#include <ustl/traits/conditional.hpp>

namespace ustl::fmt::details {

    /// A storage using an internal buffer.
    ///
    /// This storage is used when writing a single element to the output iterator
    /// is expensive.
    template <typename CharType>
    struct InternalStorage
    {
        typedef CharType    Char;

        auto begin() -> Char *
        { return buffer_; }

        static constexpr usize BUFFER_SIZE = 256 / sizeof(Char);

    private:
        Char buffer_[BUFFER_SIZE];
    };

    /// A storage writing directly to the storage.
    ///
    /// This requires the storage to be a contiguous buffer of \a Char.
    /// Since the output is directly written to the underlying storage this class
    /// is just an empty class.
    template <typename Char>
    class DirectStorage {};

    /// Write policy for directly writing to the underlying output.
    template <typename Iterator, typename Char>
    struct WriterDirect
    {
        explicit WriterDirect(Iterator out_it)
            : out_it_(out_it)
        {}

        auto out_it() -> Iterator
        {  return out_it_;  }

        auto flush(Char *, usize n) -> void
        {
            // OutputIterator can be a wrap_iter<CharT*>. Therefore the original iterator
            // is adjusted.
            out_it_ += n;
        }

    private:
        Iterator out_it_;
    };

    /// Write policy for copying the buffer to the output.
    template <typename Iterator, typename Char>
    struct WriterIterator
    {
        explicit WriterIterator(Iterator out_it)
            : out_it_{ustl::move(out_it)}
        {}

        auto out_it() && -> Iterator
        {  return ustl::move(out_it_);  }

        auto flush(Char* ptr, usize n) -> void
        {  out_it_ = algorithms::copy_n(ptr, n, ustl::move(out_it_)).out;  }

    private:
        Iterator out_it_;
    };

    /// Concept to see whether a \a _Collection is insertable.
    ///
    /// The concept is used to validate whether multiple calls to a
    /// \ref back_insert_iterator can be replace by a call to \c _Collection::insert.
    ///
    /// \note a \a _Collection needs to opt-in to the concept by specializing
    /// \ref enable_insertable.
    // template <class _Collection>
    // concept insertable =
    //     enable_insertable<_Collection> && fmt_char_type<typename _Collection::Element> &&
    //     requires(_Collection& t,
    //              add_pointer_t<typename _Collection::Element> first,
    //              add_pointer_t<typename _Collection::Element> last) { t.insert(t.end(), first, last); };

    /// Extract the container type of a \ref back_insert_iterator.
    template <typename T>
    struct BackInsertIteratorCollection
    {
        using Type = void;
    };

    template <typename Collection>
    struct BackInsertIteratorCollection<iter::BackInsertIterator<Collection>>
    {
        using Type = Collection;
    };

    /// Write policy for inserting the buffer in a container.
    template <typename Collection>
    struct WriterCollection
    {
        typedef typename Collection::Element    Char;

        explicit WriterCollection(iter::BackInsertIterator<Collection> out)
            : collection_{out.get_container()}
        {}

        auto out_it()
        {  return iter::back_inserter(*collection_);  }

        auto flush(Char* ptr, usize n) -> void
        {  collection_->insert(collection_->end(), ptr, ptr + n);  }

    private:
        Collection* collection_;
    };

    USTL_CONSTEXPR
    static bool const ENABLE_DIRECT_OUTPUT = true;

    /// Selects the type of the writer used for the output iterator.
    template <typename Iterator, typename Char>
    struct WriterSelector
    {
        typedef typename BackInsertIteratorCollection<Iterator>::Type
                Collection;

        typedef traits::ConditionalT
        <
            !traits::IsSameV<Collection, void>,
            WriterCollection<Collection>,
            traits::ConditionalT
            <
                ENABLE_DIRECT_OUTPUT,
                WriterDirect<Iterator, Char>,
                WriterIterator<Iterator, Char>
            >
        > Type;
    };

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_BUFFER_BASE_HPP

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

#ifndef USTL_FMT_DETAILS_BUFFER_FORMAT_TO_HPP
#define USTL_FMT_DETAILS_BUFFER_FORMAT_TO_HPP 1

#include <ustl/algorithms/minmax.hpp>
#include <ustl/util/move.hpp>
#include <ustl/iterator/distance_of.hpp>
#include <ustl/fmt/details/buffer_base.hpp>
#include <ustl/fmt/details/buffer_output.hpp>

namespace ustl::fmt::details {

    template <typename Iterator>
    struct FormatToNResult
    {
        Iterator out;
        iter::IterDistanceOfT<Iterator> size;
    };

    /// The base of a buffer that counts and limits the number of insertions.
    template <typename Iterator, typename CharType, bool>
    struct FormatToNBufferBase
    {
        typedef CharType    Char;
        typedef iter::IterDistanceOfT<Iterator>   Size;
        typedef typename WriterSelector<Iterator, Char>::Type   Writer;

    public:
        explicit FormatToNBufferBase(Iterator out_it, Size max_size)
            : writer_(ustl::move(out_it)),
              output_(storage_.begin(), storage_.buffer_size, this),
              max_size_(algorithms::max(Size(0), max_size))
        {}

        auto flush(Char* ptr, usize n) -> void
        {
            if (Size(size_) <= max_size_) {
                writer_.flush(ptr, algorithms::min(Size(n), max_size_ - size_));
            }

            size_ += n;
        }

    protected:
        Writer writer_;
        OutputBuffer<Char>      output_;
        InternalStorage<Char>   storage_;

        Size max_size_;
        Size size_{0};
    };

    /// The base of a buffer that counts and limits the number of insertions.
    ///
    /// This version is used when \c enable_direct_output<Iterator, Char> == true.
    ///
    /// This class limits the size available to the direct writer so it will not
    /// exceed the maximum number of code units.
    template <typename Iterator, typename Char>
    class FormatToNBufferBase<Iterator, Char, true>
    {
        using Size = iter::IterDistanceOfT<Iterator>;

    public:
        explicit FormatToNBufferBase(Iterator out_it, Size max_size)
            : output_(out_it, max_size, this),
              writer_(ustl::move(out_it)),
              max_size_(max_size)
        {
            if (max_size <= 0) [[unlikely]] {
                output_.reset(storage_.begin(), storage_.buffer_size);
            }
        }

        auto flush(Char* ptr, usize n) -> void {
            // A flush to the direct writer happens in the following occasions:
            // - The format function has written the maximum number of allowed code
            //   units. At this point it's no longer valid to write to this writer. So
            //   switch to the internal storage. This internal storage doesn't need to
            //   be written anywhere so the flush for that storage writes no output.
            // - Like above, but the next "mass write" pred would overflow the
            //   buffer. In that case the buffer is pre-emptively switched. The still
            //   valid code units will be written separately.
            // - The format_to_n function is finished. In this case there's no need to
            //   switch the buffer, but for simplicity the buffers are still switched.
            // When the max_size <= 0 the constructor already switched the buffers.
            if (size_ == 0 && ptr != storage_.begin()) {
                writer_.flush(ptr, n);
                output_.reset(storage_.begin(), storage_.buffer_size);
            } else if (size_ < max_size_) {
                // Copies a part of the internal buffer to the output up to n characters.
                // See OutputBuffer<Char>::flush_on_overflow for more information.
                Size s = algorithms::min(Size(n), max_size_ - size_);
                algorithms::copy_n(ptr, s, writer_.out_it());
                writer_.flush(ptr, s);
            }

            size_ += n;
        }

    protected:
        OutputBuffer<Char> output_;
        InternalStorage<Char> storage_;
        WriterDirect<Iterator, Char> writer_;

        Size size_;
        Size max_size_;
    };

    /// The buffer that counts and limits the number of insertions.
    template <typename Iterator, typename Char>
    struct FormatToNBuffer final
        : public FormatToNBufferBase<Iterator, Char, ENABLE_DIRECT_OUTPUT>
    {
        typedef iter::IterDistanceOfT<Iterator>     Size;
        typedef FormatToNBufferBase<Iterator, Char, ENABLE_DIRECT_OUTPUT>   Base;

    public:
        explicit FormatToNBuffer(Iterator out_it, Size max_size)
            : Base(ustl::move(out_it), max_size)
        {}

        auto make_output_iterator()
        { return this->output_.make_output_iterator(); }

        auto result() && -> FormatToNResult<Iterator>
        {
            this->output_.flush();
            return {ustl::move(this->writer_).out_it(), this->size_};
        }
    };
}

#endif // #ifndef USTL_FMT_DETAILS_BUFFER_FORMAT_TO_HPP
#ifndef USTL_FMT_DETAILS_FORMATTED_SIZE_BUFFER_H
#define USTL_FMT_DETAILS_FORMATTED_SIZE_BUFFER_H

#include <ustl/fmt/details/buffer_base.hpp>
#include <ustl/fmt/details/buffer_output.hpp>

namespace ustl::fmt::details {
    /// A buffer that counts the number of insertions.
    ///
    /// Since \ref formatted_size only needs to know the size, the output itself is
    /// discarded.
    template <typename CharType>
    struct FormattedSizeBuffer
    {
        typedef CharType                Char;
        typedef FormattedSizeBuffer     Self;

    public:
        explicit FormattedSizeBuffer(OutputBuffer<Char>)
            : storage_(),
              output_(storage_.begin(), storage_.buffer_size, this),
              size_(0)
        {}

        auto make_output_iterator()
        { return output_.make_output_iterator(); }

        auto flush(const CharType*, usize n) -> void
        { size_ += n; }

        auto result() && -> usize
        {
            output_.flush();
            return size_;
        }

    private:
        usize size_;
        OutputBuffer<Char> output_;
        InternalStorage<Char> storage_;
    };

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_FORMATTED_SIZE_BUFFER_H

#ifndef USTL_FMT_DETAILS_BUFFER_FORMAT_H
#define USTL_FMT_DETAILS_BUFFER_FORMAT_H 1

#include <ustl/fmt/details/buffer_base.hpp>
#include <ustl/fmt/details/buffer_output.hpp>

namespace ustl::fmt::details {

    /// The generic formatting buffer.
    template <typename OutputIterator, typename Char>
    struct FormatBuffer
    {
        typedef traits::ConditionalT<ENABLE_DIRECT_OUTPUT, DirectStorage<Char>, InternalStorage<Char>>
            Storage;

    public:
        explicit FormatBuffer(OutputIterator out_it)
            requires(traits::IsSameV<Storage, InternalStorage<Char>>)
            : output_(storage_.begin(), storage_.buffer_size, this),
              writer_(ustl::move(out_it))
        {}

        explicit FormatBuffer(OutputIterator out_it)
            requires(traits::IsSameV<Storage, DirectStorage<Char>>)
            : output_(out_it.base(), usize(-1), this),
              writer_(ustl::move(out_it))
        {}

        auto make_output_iterator()
        { return output_.make_output_iterator(); }

        auto flush(Char* ptr, usize n) -> void
        { writer_.flush(ptr, n); }

        auto out_it() && -> OutputIterator
        {
            output_.flush();
            return ustl::move(writer_).out_it();
        }

    private:
        Storage storage_;
        OutputBuffer<Char> output_;
        typename WriterSelector<OutputIterator, Char>::Type writer_;
    };

} // namespace ustl::fmt::details

#endif // #ifndef USTL_FMT_DETAILS_BUFFER_FORMAT_H
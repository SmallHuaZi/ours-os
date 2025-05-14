#ifndef USTL_IO_CIRCULAR_BUFFER_HPP
#define USTL_IO_CIRCULAR_BUFFER_HPP

#include <boost/circular_buffer.hpp>

namespace ustl::io {
    template <typename T, typename Alloc>
    using CircularBuffer = boost::circular_buffer<T, Alloc>;

} // namespace ustl::io

#endif // USTL_IO_CIRCULAR_BUFFER_HPP

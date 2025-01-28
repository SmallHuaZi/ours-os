#ifndef USTL_VIEWS_SPAN_H
#define USTL_VIEWS_SPAN_H

#include <boost/core/span.hpp>

namespace ustl::views {
    template <typename T>
    using Span = ::boost::span<T>;
}

#endif // #ifndef USTL_VIEWS_SPAN_H
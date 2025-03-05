#ifndef USTL_CHRONO_DURATION_HPP
#define USTL_CHRONO_DURATION_HPP

#include <boost/chrono/duration.hpp>

namespace ustl::chrono {
    using Seconds = ::boost::chrono::seconds;
    using Nanoseconds = ::boost::chrono::nanoseconds;
    using Milliseconds = ::boost::chrono::milliseconds;

    using ::boost::chrono::duration_cast;
}

#endif // #ifndef USTL_CHRONO_DURATION_HPP
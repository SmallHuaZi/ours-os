#ifndef USTL_CHRONO_DURATION_HPP
#define USTL_CHRONO_DURATION_HPP

#include <chrono>
#include <ustl/config.hpp>

namespace ustl::chrono {
    using Femtoseconds = ::std::chrono::duration<i64, ::std::femto>;
    using Picoseconds = ::std::chrono::duration<i64, ::std::pico>;
    using Nanoseconds = ::std::chrono::nanoseconds;
    using Milliseconds = ::std::chrono::milliseconds;
    using Microseconds = ::std::chrono::microseconds;;
    using Seconds = ::std::chrono::seconds;
    using Minutes = ::std::chrono::minutes;
    using Years = ::std::chrono::years;

    using ::std::chrono::duration_cast;
}

#endif // #ifndef USTL_CHRONO_DURATION_HPP
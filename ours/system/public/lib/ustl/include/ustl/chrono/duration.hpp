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

    template <typename ClockT, typename DurationT> 
    struct TimePoint: public ::std::chrono::time_point<ClockT, DurationT> {
        typedef ::std::chrono::time_point<ClockT, DurationT>  Base;
        typedef DurationT   Duration;

        USTL_CXX11_CONSTEXPR USTL_FORCEINLINE 
        TimePoint(Base const &base)
            : Base(base)
        {}

        using Base::Base;
        using Base::operator+=;
        using Base::operator-=;
        using Base::operator=;
    };

    using ::std::chrono::duration_cast;
    using ::std::chrono::time_point_cast;
}

#endif // #ifndef USTL_CHRONO_DURATION_HPP
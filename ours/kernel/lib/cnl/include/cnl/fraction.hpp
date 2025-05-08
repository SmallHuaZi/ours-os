#ifndef INCLUDE_CNL_FRACTION_HPP
#define INCLUDE_CNL_FRACTION_HPP

#include <cnl/all.h>
#include <cnl/fraction.h>
#include <cnl/static_number.h>

namespace cnl {
    // template<fixed_point Numerator = int, fixed_point Denominator = Numerator>
    // using Fraction = fraction<Numerator, Denominator>;

    template<int Exponent = 0, int Radix = 2>
    using Power = power<Exponent, Radix>;

    template <typename Rep, typename Scale = Power<>>
    using Fraction = scaled_integer<Rep, Scale>;
}

#endif // INCLUDE_CNL_FRACTION_HPP

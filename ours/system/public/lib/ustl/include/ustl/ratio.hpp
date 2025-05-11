#ifndef INCLUDE_USTL_RATIO_HPP
#define INCLUDE_USTL_RATIO_HPP

#include <boost/rational.hpp>

namespace ustl {
    template <typename Int>
    using Ratio = ::boost::rational<Int>;

    // Calculates the greatest common divisor of two values.
    template <typename Integer>
    auto calc_binary_gcd(Integer a, Integer b) -> Integer {
        // Remove and count the common factors of 2.
        Integer twos;
        for (twos = 0; ((a | b) & 1) == 0; ++twos) {
            a >>= 1;
            b >>= 1;
        }

        // Get rid of the non-common factors of 2 in a. a is non-zero, so this
        // terminates.
        while ((a & 1) == 0) {
            a >>= 1;
        }

        do {
            // Get rid of the non-common factors of 2 in b. b is non-zero, so this
            // terminates.
            while ((b & 1) == 0) {
                b >>= 1;
            }

            // Apply the Euclid subtraction method.
            if (a > b) {
                std::swap(a, b);
            }

            b = b - a;
        } while (b != 0);

        // Multiply in the common factors of two.
        return a << twos;
    }

    template <typename Integer>
    auto reduce(Integer &numerator, Integer &denominator) -> void {
        if (numerator == 0) {
            denominator = 1;
            return;
        }

        auto gcd = calc_binary_gcd(numerator, denominator);
        if (gcd == 1) {
            return;
        }

        numerator /= gcd;
        denominator /= gcd;
    }

} // namespace ustl

#endif // INCLUDE_USTL_RATIO_HPP

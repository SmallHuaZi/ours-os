#ifndef INCLUDE_USTL_RATIO_HPP
#define INCLUDE_USTL_RATIO_HPP

#include <boost/rational.hpp>

namespace ustl {
    template <typename Int>
    using Ratio = ::boost::rational<Int>;

} // namespace ustl

#endif // INCLUDE_USTL_RATIO_HPP

/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///

#ifndef USTL_COLLECTIONS_INTRUSIVE_OPTIONS_HPP
#define USTL_COLLECTIONS_INTRUSIVE_OPTIONS_HPP 1

#include <boost/intrusive/options.hpp>

namespace ustl::collections::intrusive {
    template <typename Size>
    using SizeType = boost::intrusive::size_type<Size>;

    template <bool Enable>
    using ConstantTimeSize = boost::intrusive::constant_time_size<Enable>;

    template <bool Enable>
    using CacheLast = boost::intrusive::cache_last<Enable>;

    template <typename HeaderHolder>
    using HeaderHolderType = boost::intrusive::header_holder_type<HeaderHolder>;

} // namespace ustl::collections::intrusive

#endif // #ifndef USTL_COLLECTIONS_INTRUSIVE_OPTIONS_HPP
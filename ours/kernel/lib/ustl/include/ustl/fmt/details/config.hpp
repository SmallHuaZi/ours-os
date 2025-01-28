#ifndef USTL_FMT_DETAILS_CONFIG_H
#define USTL_FMT_DETAILS_CONFIG_H

#include <ustl/config.hpp>

#define USTL_FMT ustl::fmt::
#define USTL_VFMT ustl::fmt::details::

namespace ustl::fmt::details {

    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto use_packed_format_arg_store(...) -> bool
    {  return false;  }
}

#endif
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

#ifndef USTL_CONFIG_NAMESPACE_HPP
#define USTL_CONFIG_NAMESPACE_HPP 1

#include <ustl/config/macro_abi.hpp>

namespace ustl {
    namespace fn {} // namespace ustl::function
    namespace views {} // namespace ustl::views 
    namespace collections {
        namespace details {
        } // namespace ustl::collections::details
        namespace test {
        } // namespace ustl::collections::test
        namespace intrusive {
            namespace details {
                using namespace ustl::collections::details;
            } // namespace ustl::collections::intrusive::details
            namespace test {
                using namespace ustl::collections::test;
            } // namespace ustl::collections::intrusive::test
        } // namespace ustl::collections::intrusive
    } // namespace ustl::collections

    namespace traits {} // namespace ustl::traits
    
} // namespace ustl

#endif // #ifndef USTL_DETAILS_NAMESPACE_HPP
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

#ifndef USTL_COLLECTIONS_INTRUSIVE_SET_HPP
#define USTL_COLLECTIONS_INTRUSIVE_SET_HPP 1

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/set.hpp>

#include <ustl/collections/intrusive/set_hook.hpp>

namespace ustl::collections::intrusive {
    template <typename... T>
    using Set = boost::intrusive::set<T...>;

    template <typename... T>
    using MultiSet = boost::intrusive::multiset<T...>;

} // namespace ustl::collections::intrusive

#ifndef USTL_DECLARE_OPTIONS
#   define USTL_DECLARE_COMPARE_OPTIONS(COMPARER, ALIAS) \
        typedef boost::intrusive::compare<COMPARER>  ALIAS;
#endif

#ifndef USTL_DECLARE_HOOK_OPTION
#   define USTL_DECLARE_HOOK_OPTION(CLASS, MEMBER, ALIAS) \
       typedef ustl::collections::intrusive::MemberHook<CLASS, decltype(MEMBER), &CLASS::MEMBER>     ALIAS;
#endif

#ifndef USTL_DECLARE_MULTISET
#   define USTL_DECLARE_MULTISET(CLASS, ALIAS, ...) \
        typedef ustl::collections::intrusive::MultiSet<CLASS, __VA_ARGS__>\
            ALIAS;
#endif

#ifndef USTL_DECLARE_MULTISET_TEMPLATE
#   define USTL_DECLARE_MULTISET_TEMPLATE(OWNER, ALIAS, ...)                                    \
        template <typename... Options>                                                      \
        using ALIAS = ustl::collections::intrusive::MultiSet<OWNER, Options..., __VA_ARGS__>;
#endif


#endif // #ifndef USTL_COLLECTIONS_INTRUSIVE_SET_HPP
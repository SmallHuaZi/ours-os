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

#ifndef USTL_COLLECTIONS_INTRUSIVE_LIST_HOOK_HPP
#define USTL_COLLECTIONS_INTRUSIVE_LIST_HOOK_HPP 1

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <ustl/collections/intrusive/options.hpp>

namespace ustl::collections::intrusive {
    using namespace boost::intrusive;

    template <typename Parent, typename Hook, Hook Parent::*PtrToMember>
    using MemberHook = member_hook<Parent, Hook, PtrToMember>;

    template <typename... T>
    using ListBaseHook = list_member_hook<T...>;

    template <typename... T>
    using ListMemberHook = list_member_hook<T...>;

} // namespace ustl::collections::intrusive

#ifndef USTL_DECLARE_HOOK_OPTION
#   define USTL_DECLARE_HOOK_OPTION(OWNER, MEMBER, ALIAS) \
        typedef ustl::collections::intrusive::MemberHook<OWNER, \
                decltype(OWNER::MEMBER), &OWNER::MEMBER> \
            ALIAS;
#endif

#endif // #ifndef USTL_COLLECTIONS_INTRUSIVE_LIST_HOOK_HPP
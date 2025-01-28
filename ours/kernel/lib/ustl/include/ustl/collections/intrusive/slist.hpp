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

#ifndef USTL_COLLECTIONS_INTRUSIVE_SLIST_HPP
#define USTL_COLLECTIONS_INTRUSIVE_SLIST_HPP 1

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/slist_hook.hpp>
#include <boost/intrusive/slist.hpp>

namespace ustl::collections::intrusive {
    using namespace boost::intrusive;

    template <typename Parent, typename MemberHook, MemberHook Parent::*PtrToMember>
    using MemberHook = member_hook<Parent, MemberHook, PtrToMember>;

    template <typename... T>
    using SlistBaseHook = slist_base_hook<T...>;

    template <typename... T>
    using SlistMemberHook = slist_member_hook<T...>;

    template<class T, class ...Options>
    class Slist
        : public slist<T, Options...>
    {
        typedef slist<T, Options...>     Base;
    public:
        typedef typename Base::value_type               Element;
        typedef typename Base::pointer                  PtrMut;
        typedef typename Base::const_pointer            Ptr;
        typedef typename Base::reference                RefMut;
        typedef typename Base::const_reference          Ref;
        typedef typename Base::iterator                 IterMut;
        typedef typename Base::const_iterator           Iter;

        using Base::Base;
    };

} // namespace ustl::collections::intrusive

#endif // USTL_COLLECTIONS_INTRUSIVE_SLIST_HPP
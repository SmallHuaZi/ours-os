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

#include <boost/intrusive/slist.hpp>

#include <ustl/collections/intrusive/options.hpp>
#include <ustl/collections/intrusive/slist_hook.hpp>

namespace ustl::collections::intrusive {
    template<class T, class ...Options>
    class Slist
        : public internal::slist<T, Options...>
    {
        typedef internal::slist<T, Options...>     Base;
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

#ifndef USTL_DECLARE_SLIST
#   define USTL_DECLARE_SLIST(OWNER, ALIAS, ...) \
        typedef ustl::collections::intrusive::Slist<OWNER, __VA_ARGS__>\
            ALIAS;
#endif

#ifndef USTL_DECLARE_SLIST_TEMPLATE
#   define USTL_DECLARE_SLIST_TEMPLATE(OWNER, ALIAS, ...)                                    \
        template <typename... Options>                                                      \
        using ALIAS = ustl::collections::intrusive::Slist<OWNER, Options..., __VA_ARGS__>;
#endif

#endif // USTL_COLLECTIONS_INTRUSIVE_SLIST_HPP
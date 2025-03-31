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

#ifndef USTL_COLLECTIONS_INTRUSIVE_LIST_HPP
#define USTL_COLLECTIONS_INTRUSIVE_LIST_HPP 1

#include <boost/intrusive/list.hpp>
#include <ustl/collections/intrusive/list_hook.hpp>

namespace ustl::collections::intrusive {
    using namespace boost::intrusive;

    template<class T, class ...Options>
    class List
        : public list<T, Options...>
    {
        typedef list<T, Options...>     Base;
    public:
        typedef typename Base::value_type               Element;
        typedef typename Base::pointer                  PtrMut;
        typedef typename Base::const_pointer            Ptr;
        typedef typename Base::reference                RefMut;
        typedef typename Base::const_reference          Ref;
        typedef typename Base::iterator                 IterMut;
        typedef typename Base::const_iterator           Iter;
        typedef typename Base::reverse_iterator         RevIterMut;
        typedef typename Base::const_reverse_iterator   RevIter;

        using Base::Base;
    };

} // namespace ustl::collections::intrusive

#ifndef USTL_DECLARE_LIST
#   define USTL_DECLARE_LIST(OWNER, ALIAS, ...) \
        typedef ustl::collections::intrusive::List<OWNER, __VA_ARGS__>\
            ALIAS;
#endif

#ifndef USTL_DECLARE_LIST_TEMPLATE
#   define USTL_DECLARE_LIST_TEMPLATE(OWNER, ALIAS, ...)                                    \
        template <typename... Options>                                                      \
        using ALIAS = ustl::collections::intrusive::List<OWNER, Options..., __VA_ARGS__>;
#endif

#endif // #ifndef USTL_COLLECTIONS_INTRUSIVE_LIST_HPP
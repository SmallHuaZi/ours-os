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

#ifndef USTL_COLLECTIONS_INTRUSIVE_ANY_HOOK_HPP
#define USTL_COLLECTIONS_INTRUSIVE_ANY_HOOK_HPP 1

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/any_hook.hpp>

namespace ustl::collections::intrusive {
    using namespace boost::intrusive;

    template <typename Parent, typename Hook, Hook Parent::*PtrToMember>
    using MemberHook = member_hook<Parent, Hook, PtrToMember>;

    template <typename... T>
    using AnyBaseHook = any_base_hook<T...>;

    template <typename... T>
    using AnyMemberHook = any_member_hook<T...>;

    template <typename AnyHook>
    using AnyToSetHook = ::boost::intrusive::any_to_set_hook<AnyHook>;

    template <typename AnyHook>
    using AnyToAvlSetHook = ::boost::intrusive::any_to_avl_set_hook<AnyHook>;

    template <typename PtrBox>
    using AnyNode = ::boost::intrusive::any_node<PtrBox>;

    template <typename PtrBox>
    struct AnyListNodeTraits {
       typedef AnyNode<PtrBox>          Node;
       typedef typename Node::node_ptr        node_ptr;
       typedef typename Node::const_node_ptr  const_node_ptr;

       BOOST_INTRUSIVE_FORCEINLINE static node_ptr get_next(const_node_ptr n)
       {  return n->node_ptr_2;  }

       BOOST_INTRUSIVE_FORCEINLINE static void set_next(node_ptr n, node_ptr next)
       {  n->node_ptr_2 = next;  }

       BOOST_INTRUSIVE_FORCEINLINE static node_ptr get_previous(const_node_ptr n)
       {  return n->node_ptr_3;  }

       BOOST_INTRUSIVE_FORCEINLINE static void set_previous(node_ptr n, node_ptr prev)
       {  n->node_ptr_3 = prev;  }
    };

    template<class BasicHook>
    struct AnyToListHook
        : public detail::any_to_some_hook<BasicHook, AnyListNodeTraits>
    {};

    template <typename AnyHook>
    using AnyToSlistHook = ::boost::intrusive::any_to_slist_hook<AnyHook>;

} // namespace ustl::collections::intrusive

#ifndef USTL_DECLARE_HOOK_OPTION
#   define USTL_DECLARE_HOOK_OPTION(OWNER, MEMBER, ALIAS) \
        typedef ustl::collections::intrusive::MemberHook<OWNER, \
                decltype(OWNER::MEMBER), &OWNER::MEMBER> \
            ALIAS;
#endif

#endif // #ifndef USTL_COLLECTIONS_INTRUSIVE_ANY_HOOK_HPP
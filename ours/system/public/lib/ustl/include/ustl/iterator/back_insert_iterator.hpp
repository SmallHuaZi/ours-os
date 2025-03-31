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

#ifndef USTL_ITERATOR_BACK_INSERT_ITERATOR_HPP
#define USTL_ITERATOR_BACK_INSERT_ITERATOR_HPP 1

#include <ustl/util/move.hpp>
#include <ustl/iterator/category.hpp>
#include <ustl/iterator/details/iter_types.hpp>

#include <ustl/mem/address_of.hpp>

namespace ustl::iterator {
namespace details {
    template<typename CollectionType>
    class BackInsertIterator
        : public IteratorTypes<ReadOnly<false>, CollectionType>
    {
        typedef BackInsertIterator  Self;
        typedef IteratorTypes<ReadOnly<false>, CollectionType>  Base;

    public:
        typedef isize                   Distance;
        typedef CollectionType          Collection;
        typedef typename Base::Element  Element;
        typedef typename Base::Ptr      Ptr;
        typedef typename Base::PtrMut   PtrMut;
        typedef typename Base::Ref      Ref;
        typedef typename Base::RefMut   RefMut;
        typedef typename Base::SafeRef  SafeRef;
        typedef typename Base::SafePtr  SafePtr;

        /// The only way to create this %iterator is with a collection_.
        explicit USTL_CXX20_CONSTEXPR 
        BackInsertIterator(Collection &x)
            : collection_(mem::address_of(x)) 
        {}

        /// 
        /// @param  value  An instance of whatever type
        ///                collection_type::const_reference is; presumably a
        ///                reference-to-const T for collection_<T>.
        /// @return  This %iterator, for chained operations.
        /// 
        /// This kind of %iterator doesn't really have a @a position in the
        /// collection_ (you can think of the position as being permanently at
        /// the end, if you like).  Assigning a value to the %iterator will
        /// always append the value to the end of the collection_.
        USTL_CXX20_CONSTEXPR
        auto operator=(Ref value) -> BackInsertIterator &
        {
	        collection_->push_back(value);
	        return *this;
        }

        USTL_CXX20_CONSTEXPR
        auto operator=(Element &&value)
            -> BackInsertIterator &
        {
	        collection_->push_back(ustl::move(value));
	        return *this;
        }

        /// Simply returns *this.
        USTL_CXX20_CONSTEXPR
        auto operator*() -> BackInsertIterator &
        { return *this; }

        /// Simply returns *this.  (This %iterator does not @a move.)
        USTL_CXX20_CONSTEXPR
        auto operator++() -> BackInsertIterator &
        { return *this; }

        /// Simply returns *this.  (This %iterator does not @a move.)
        USTL_CXX20_CONSTEXPR
        auto operator++(int) -> BackInsertIterator
        { return *this; }

    protected:
        Collection *collection_;
    };

}
    using details::BackInsertIterator;

    template <typename Collection>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto back_inserter(Collection &collection) -> BackInsertIterator<Collection>
    {  return BackInsertIterator<Collection>(collection);  }
}

#endif // #ifndef USTL_ITERATOR_BACK_INSERT_ITERATOR_HPP
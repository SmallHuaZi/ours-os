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
#ifndef USTL_ITERATOR_REVERSE_ITERATOR_HPP
#define USTL_ITERATOR_REVERSE_ITERATOR_HPP 1

#include <ustl/iterator/traits.hpp>

namespace ustl::iterator {
    template <typename Iterator>
    struct RevIter
    {
        typedef RevIter Self;
        typedef IterTraits<Iterator>                IterTraits;
        typedef typename IterTraits::Element        Element;
        typedef typename IterTraits::Category       Category;
        typedef typename IterTraits::Pointer        Pointer;
        typedef typename IterTraits::Reference      Reference;
    public:
        USTL_FORCEINLINE USTL_CONSTEXPR 
        RevIter() USTL_NOEXCEPT
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        RevIter(Iterator iter) USTL_NOEXCEPT
            : _iter(iter)
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator++() & USTL_NOEXCEPT -> Self &
        {  
            --_iter;
            return *this;  
        }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator--() & USTL_NOEXCEPT -> Self &
        {  
            ++_iter;
            return *this;  
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator+=(isize n) USTL_NOEXCEPT -> Self &
        {  
            _iter -= n;  
            return *this;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto operator-=(isize n) USTL_NOEXCEPT -> Self &
        {  
            _iter += n;  
            return *this;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator++(int) & USTL_NOEXCEPT -> Self 
        {  return Self(_iter--);  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator--(int) & USTL_NOEXCEPT -> Self 
        {  return Self(_iter++);  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator->() USTL_NOEXCEPT -> Pointer
        {  return _iter.operator->();  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator*() USTL_NOEXCEPT -> Reference
        {  
	        Iterator tmp = _iter;
	        return *--tmp;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        friend auto operator!=(Self const &x, Self const &y) -> bool {
            return x._iter != y._iter;
        }
    private:
        Iterator  _iter;
    };
    
} // namespace ustl::iterator

#endif // #ifndef USTL_ITERATOR_REVERSE_ITERATOR_HPP
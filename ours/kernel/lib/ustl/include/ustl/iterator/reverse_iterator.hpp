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

namespace ustl::iter {
    template <typename T>
    struct RevIter
    {
        typedef T ProtoIter; 
        typedef RevIter Self;
        typedef typename ProtoIter::Ptr             Ptr;
        typedef typename ProtoIter::Ref             Ref;
        typedef typename ProtoIter::RefMut          RefMut;
        typedef typename ProtoIter::PtrMut          PtrMut;
        typedef typename ProtoIter::Element         Element;
        typedef typename ProtoIter::Category        Category;
        typedef typename ProtoIter::SafePtr         SafePtr;
        typedef typename ProtoIter::SafeRef         SafeRef;
        typedef typename ProtoIter::MutableIter     ProtoMutableIter;
        typedef RevIter<ProtoMutableIter>           MutableIter; 
        typedef typename ProtoIter::ConstantIter    ProtoConstantIter;
        typedef RevIter<ProtoConstantIter>          ConstantIter; 

    public:
        USTL_FORCEINLINE USTL_CONSTEXPR 
        RevIter() USTL_NOEXCEPT
        {}

        USTL_FORCEINLINE USTL_CONSTEXPR 
        RevIter(ProtoIter iter) USTL_NOEXCEPT
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
        auto operator->() USTL_NOEXCEPT -> SafePtr
        {  return this->to_value_ptr();  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator*() USTL_NOEXCEPT -> SafeRef
        {  return *this->to_value_ptr();  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto to_mut() USTL_NOEXCEPT -> MutableIter 
        {  return  MutableIter(_iter.to_mut());  }

        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto to_value_ptr() USTL_NOEXCEPT -> SafePtr 
        {  return _iter.to_value_ptr();  }

    private:
        ProtoIter  _iter;
    };
    
} // namespace ustl::iter

#endif // #ifndef USTL_ITERATOR_REVERSE_ITERATOR_HPP
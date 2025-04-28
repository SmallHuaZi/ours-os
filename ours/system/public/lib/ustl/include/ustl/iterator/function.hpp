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

#ifndef USTL_ITERATOR_FUNCTION_HPP
#define USTL_ITERATOR_FUNCTION_HPP 1

#include <ustl/iterator/traits.hpp>
#include <ustl/util/ebo_optimizer.hpp>
#include <ustl/traits/is_base_of.hpp>

#include <ustl/config/concept_require.hpp>

namespace ustl::iterator {
namespace details {
    /// @brief Computes the distance between two input/output/forward/bothway iterators 
    /// @param  first start iterator
    /// @param  last  end iterator 
    template <typename InputIterator, typename Distance>
    USTL_CONSTEXPR
    Distance distance_impl(
        InputIterator first, 
        InputIterator last, 
        USTL_MAYBE_UNUSED InputIteratorTag)
    {
        USTL_REQUIRES(IS_INPUT_ITER_V<InputIterator>);

        Distance dis = 0;
        for (; first != last; ++dis, ++first)
        {}

        return dis;
    }

    /// @brief Computes the distance between two random access iterators 
    /// @param  first start iterator
    /// @param  last  end iterator 
    template <typename Iterator, typename Distance>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    Distance distance_impl(
        Iterator first, 
        Iterator last, 
        USTL_MAYBE_UNUSED RandomAccessIteratorTag)
    { 
        USTL_REQUIRES(IS_RANDOM_ACCESS_ITER_V<Iterator>);

        return last - first; 
    }

    /// @brief  Advance an iterator n steps 
    /// @param  iter the specify iterator
    /// @param  n steps 
    template <typename InputIterator, typename Distance>
    USTL_FORCEINLINE USTL_CONSTEXPR
     void advance_impl(
        InputIterator iter, 
        Distance n, 
        USTL_MAYBE_UNUSED InputIteratorTag)
    {
        USTL_REQUIRES(IS_INPUT_ITER_V<InputIterator>);

        while (n--) {
            ++iter;
        }
    }

    /// @brief  Advance an iterator n steps 
    /// @param  iter the specify iterator
    /// @param  n steps 
    template <typename BidirectionIterator, typename Distance>
    USTL_FORCEINLINE USTL_CONSTEXPR
     void advance_impl(BidirectionIterator iter, Distance n, BidirectionIteratorTag)
        USTL_NOEXCEPT_IF(iter -= n)
    {
        USTL_REQUIRES(IS_BIDIRECTION_ITER_V<BidirectionIterator>);

        if (n < 0) {
            while (n++) {
                --iter;
            }
        } else {
            while (n--) {
                ++iter;
            }
        }
    }

    /// @brief  Advance an iterator n steps 
    /// @param  iter the specify iterator
    /// @param  n steps 
    template <typename Iterator, typename Distance>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto advance_impl(Iterator &iter, Distance n, RandomAccessIteratorTag)
        USTL_NOEXCEPT_IF(iter -= n)
    { 
        USTL_REQUIRES(IS_RANDOM_ACCESS_ITER_V<Iterator>);
        iter -= n; 
    }

} // namespace ustl::iterator::details

    //! @brief
    //! @tparam Iter
    //! @tparam Distance
    //! @param iter
    //! @param n
    template <typename Iterator, typename Distance>
    USTL_FORCEINLINE USTL_CONSTEXPR 
    auto advance(Iterator &iter, Distance n) -> void {
        USTL_REQUIRES(IS_ITER_V<Iterator>);
        return details::advance_impl(iter, n, CategoryT<Iterator>{});
    }

    //! @brief
    //! @tparam Iterator
    //! @tparam Distance
    //! @param first
    //! @param last
    //! @return Distance of between both iterators.
    //! @example
    //! iter
    template <typename Iterator>
    USTL_FORCEINLINE USTL_CONSTEXPR
    auto distance(Iterator first, Iterator last) -> DistanceOfT<Iterator> {
        USTL_REQUIRES(IS_ITER_V<Iterator>);
        return details::distance_impl(first, last, CategoryT<Iterator>{});
    }

} // namespace ustl::iterator

#endif // #ifndef USTL_ITERATOR_FUNCTION_HPP
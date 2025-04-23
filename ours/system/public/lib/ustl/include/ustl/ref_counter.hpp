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
#ifndef USTL_REF_COUNTER_HPP
#define USTL_REF_COUNTER_HPP 1

#include <ustl/config.hpp>

#include <ustl/placeholders.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/util/noncopyable.hpp>
#include <ustl/util/ebo_optimizer.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_base_of.hpp>

#include <ustl/mem/recycler.hpp>
#include <ustl/mem/allocator.hpp>

namespace ustl {
    template <typename T>
    struct RcEvictor;

    template <typename T, typename A, bool = traits::IsSameV<A, Monostate>>
    struct RcReclaimer;

    template <typename T, typename A = Monostate, typename Size = usize>
    class RefCounter;

    template <typename T, typename A>
    struct RcReclaimer<T, A, true> {
        USTL_FORCEINLINE
        static auto destory(T *object) -> void {
            RcReclaimer<T, A>::reclaim(object);
        }
    };

    template <typename T, typename A>
    struct RcReclaimer<T, A, false> {
        USTL_FORCEINLINE
        static auto destory(T *object) -> void {
            object->get_allocator().destory(object);
            object->get_allocator().deallocate(object);
        }
    };

    /// This class defines a atomic reference counter designed to be adopted by 
    /// `Rc<T>` and `Weak<T>`. Additionally, user-defined classes can inherit from 
    /// it to implement an intrusive reference counting mechanism, enabling the use 
    /// of `Rc<T>` in environments without managed memory, such as freestanding or 
    /// embedded systems.
    ///
    /// `Disposer` is a function object type, and this class will inherit from it 
    /// to implement a stateful disposer. When the strong reference count (i.e., 
    /// the count associated with Rc<T>) reaches zero, it will be invoked by Rc<T> 
    /// to destroy the object it manages.
    ///
    /// If a user-defined class inherits from this class and wishes to provide 
    /// a disposer, please ensure that the code responsible for recycling the managed 
    /// object is not included. This is to avoid double memory deallocation, which can 
    /// lead to undefined behavior.
    ///
    /// "Size" is a primitive integer type. Users can provide alternative types to 
    /// suit their specific requirements.
    ///
    /// The use of this class in intrusive reference counting designs should 
    /// follow a pattern similar to the example code below
    /// ```cpp
    /// struct Window: RefCounter<>
    /// {};
    ///
    /// //  
    /// auto main() {
    ///     Window window; 
    ///     Rc<Window> rc = ustl::make_rc(window);
    /// }
    ///
    template <typename T, typename A, typename Size>
    class RefCounter: public EboOptimizer<A> {
        typedef RefCounter                  Self;
        typedef ustl::sync::Atomic<Size>    AtomciSize;
    protected:
        USTL_FORCEINLINE
        RefCounter() USTL_NOEXCEPT
            : weak_counter_(0),
              strong_counter_(1)
        {}

        USTL_FORCEINLINE
        virtual ~RefCounter() USTL_NOEXCEPT {
            USTL_ASSERT(weak_counter_ == 0 || weak_counter_ == PRE_ENABLE_SENTINEL);
            USTL_ASSERT(strong_counter_ == 0 || strong_counter_  == PRE_ENABLE_SENTINEL);
        }

        USTL_NO_MOVEABLE_AND_COPYABLE(RefCounter);
    private:
        template <typename, typename, typename>
        friend class Rc;

        template <typename, typename, typename>
        friend class Weak;

        USTL_FORCEINLINE
        auto inc_strong_ref() USTL_NOEXCEPT -> void 
        {  strong_counter_  += 1;  }

        USTL_FORCEINLINE
        auto dec_strong_ref() USTL_NOEXCEPT -> void
        {  strong_counter_  -= 1;  }

        USTL_FORCEINLINE
        auto inc_weak_ref() USTL_NOEXCEPT -> void 
        {  weak_counter_  += 1;  }

        USTL_FORCEINLINE
        auto dec_weak_ref() USTL_NOEXCEPT -> void 
        {  weak_counter_ -= 1;  }

        USTL_FORCEINLINE
        auto weak_count() USTL_NOEXCEPT -> Size
        {  return weak_counter_;  }

        USTL_FORCEINLINE
        auto strong_count() USTL_NOEXCEPT -> Size
        {  return strong_counter_;  }
    
        /// It is provided to support make_*() functions.
        USTL_FORCEINLINE
        auto enable_strong() USTL_NOEXCEPT -> void
        {  strong_counter_  = 1;  }

        USTL_FORCEINLINE
        auto enable_weak() USTL_NOEXCEPT -> void
        {  weak_counter_  = 1;  }

        USTL_CONSTEXPR
        static u32 const kPreEnableSentinel = 0xC0000000;

        mutable AtomciSize weak_counter_;
        mutable AtomciSize strong_counter_;
    };

} // namespace ustl

#endif // #ifndef USTL_REF_COUNTER_HPP
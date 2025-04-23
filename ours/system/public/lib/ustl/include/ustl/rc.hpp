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
#ifndef USTL_RC_HPP
#define USTL_RC_HPP 1

#include <ustl/ref_counter.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/traits/is_same.hpp>

#include <ustl/mem/allocator_traits.hpp>

namespace ustl {
    template <typename, typename A = Monostate, typename S = usize>
    class Rc;

    template <typename, typename A = Monostate, typename S = usize>
    class Weak;

    template <typename T, typename A, typename S>
    class Rc {
        typedef Rc      Self;
    public:
        typedef T const *   Ptr;
        typedef T const &   Ref;
        typedef T *         PtrMut;
        typedef T &         RefMut;
        typedef T           Element;

        typedef RefCounter<T, A, S>    Counter;
        typedef Weak<T,  A, S>         ThisWeak;

        template <typename, typename, typename>
        friend class Rc;

        USTL_FORCEINLINE
        Rc() USTL_NOEXCEPT
            : pointer_(nullptr),
              counter_(nullptr)
        {}

        USTL_FORCEINLINE
        Rc(decltype(nullptr)) USTL_NOEXCEPT
            : pointer_(nullptr),
              counter_(nullptr)
        {}

        // Downcast is allowed but need to be give out explicitly.
        template <typename U>
            requires traits::IsBaseOfV<U, T>
        USTL_FORCEINLINE
        explicit Rc(U *ptr) USTL_NOEXCEPT
            : pointer_(static_cast<T *>(ptr)),
              counter_() {
            counter_->enable_strong();
        }

        // Upcast is correct and there should not be any problem.
        template <typename U>
            requires traits::IsBaseOfV<T, U>
        USTL_FORCEINLINE
        Rc(U *ptr) USTL_NOEXCEPT
            : pointer_(static_cast<T *>(ptr)),
              counter_() {
            counter_->enable_strong();
        }

        USTL_FORCEINLINE
        Rc(PtrMut ptr, Counter *counter) USTL_NOEXCEPT
            : pointer_(ptr),
              counter_(counter)
        {}

        template <typename U>
            requires traits::IsBaseOfV<T, U>
        USTL_FORCEINLINE
        Rc(Rc<U, A, S> const &other) USTL_NOEXCEPT
            : pointer_(other.pointer_),
              counter_(reinterpret_cast<Counter *>(other.counter_))
        {
            if (counter_) {
                counter_->inc_strong_ref();
            }
        }

        template <typename U>
            requires traits::IsBaseOfV<T, U>
        USTL_FORCEINLINE
        Rc(Rc<U, A, S> &&other) USTL_NOEXCEPT
            : pointer_(other.pointer_),
              counter_(other.counter_)
        {
            other.counter_ = nullptr;
            other.pointer_ = nullptr;
        }

        template <typename U>
        USTL_FORCEINLINE
        Rc(Weak<U, A, S> const &weak) USTL_NOEXCEPT
            : pointer_(weak.pointer_),
              counter_(weak.counter_)
        {
            if (counter_) {
                counter_->inc_strong_ref();
            }
        }

        template <typename U>
        USTL_FORCEINLINE
        Rc(Weak<U, A, S> &&weak) USTL_NOEXCEPT
            : pointer_(weak.pointer_),
              counter_(weak.counter_)
        {
            if (counter_) {
                counter_->inc_strong_ref();
                counter_->dec_weak_ref();
                weak.pointer_ = nullptr;
                weak.counter_ = nullptr;
            }
        }

        USTL_FORCEINLINE
        ~Rc() USTL_NOEXCEPT {
            destory();
        }

        USTL_FORCEINLINE
        auto operator=(Self const &other) USTL_NOEXCEPT -> Self & {
            destory();
            pointer_ = other.pointer_;
            counter_ = other.counter_;
            if (counter_) {
                counter_->inc_strong_ref();
            }

            return *this;
        }

        /// Upcast
        template <typename U>
            requires traits::IsBaseOfV<T, U>
        USTL_FORCEINLINE
        auto operator=(Rc<U, A, S> const &other) USTL_NOEXCEPT -> Self & {
            destory();
            pointer_ = other.pointer_;
            counter_ = other.counter_;
            if (counter_) {
                counter_->inc_strong_ref();
            }
            return *this;
        }

        /// Upcast
        template <typename U>
            requires traits::IsBaseOfV<T, U>
        USTL_FORCEINLINE
        auto operator=(Rc<U, A, S> &&other) USTL_NOEXCEPT -> Self & {
            destory();
            pointer_ = other.pointer_;
            counter_ = other.counter_;
            other.destory();
            return *this;
        }

        template <typename Ptr>
        auto reset(Ptr ptr) -> void;

        USTL_FORCEINLINE
        auto take() USTL_NOEXCEPT -> T * {
            auto t = pointer_;
            pointer_ = 0;
            counter_ = 0;
            return t;
        }

        USTL_FORCEINLINE
        auto is_unique() USTL_NOEXCEPT -> bool
        {  return counter_->strong_count() == 1;  }

        USTL_FORCEINLINE
        auto use_count() USTL_NOEXCEPT -> usize
        {  return counter_->strong_count();  }

        USTL_FORCEINLINE
        auto as_ptr_mut() USTL_NOEXCEPT -> T *
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto as_ref_mut() USTL_NOEXCEPT -> T &
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto as_ptr() USTL_NOEXCEPT -> T const * {  
            return pointer_;
        }

        USTL_FORCEINLINE
        auto as_ref() USTL_NOEXCEPT -> T const &
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator*() const USTL_NOEXCEPT -> Ref
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator->() const USTL_NOEXCEPT -> Ptr
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto operator*() USTL_NOEXCEPT -> RefMut
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator->() USTL_NOEXCEPT -> PtrMut
        {  return pointer_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        operator bool() USTL_NOEXCEPT 
        {  return pointer_ != nullptr;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto destory() -> void {
            if (counter_) {
                if (counter_->strong_count() == 1) {
                    ThisWeak weak(pointer_, counter_);
                    // counter_->disposer()(pointer_);
                } else {
                    counter_->dec_strong_ref();
                }

                counter_ = 0;
                pointer_ = 0;
            }
        }
    private:
        template <typename, typename, typename>
        friend class Weak;

        PtrMut   pointer_;
        Counter *counter_;
    };

    template <typename T, typename A, typename S>
    class Weak {
        typedef Weak    Self;
    public:
        typedef T const *   Ptr;
        typedef T const &   Ref;
        typedef T *         PtrMut;
        typedef T &         RefMut;
        typedef T           Element;

        typedef Rc<T, A, S>            ThisRc;
        typedef RefCounter<T, A, S>    Counter;

        USTL_FORCEINLINE USTL_CONSTEXPR
        Weak(Self const &other) USTL_NOEXCEPT
            : pointer_(other.pointer_),
              counter_(other.counter_)
        {
            if (counter_) {
                counter_->inc_weak_ref();
            }
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        Weak(Self &&other) USTL_NOEXCEPT
            : pointer_(other.pointer_),
              counter_(other.counter_)
        {
            other.pointer_ = nullptr;
            other.counter_ = nullptr;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        ~Weak() USTL_NOEXCEPT
        {
            if (counter_) {
                if (counter_->weak_count() == 1) {
                    // AllocatorTraits::deallocate(counter_->allocator(), pointer_);
                } else {
                    counter_->dec_weak_ref();
                }
            }
        }
    
        USTL_FORCEINLINE
        auto upgrade() -> ThisRc
        {  return ThisRc(*this);  }

        USTL_FORCEINLINE
        auto use_count() USTL_NOEXCEPT -> usize
        {  return counter_->weak_count();  }

        USTL_FORCEINLINE
        auto as_ptr_mut() USTL_NOEXCEPT -> T *
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto as_ref_mut() USTL_NOEXCEPT -> T &
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto as_ptr() USTL_NOEXCEPT -> T const *
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto as_ref() USTL_NOEXCEPT -> T const &
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator*() const USTL_NOEXCEPT -> Ref
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator->() const USTL_NOEXCEPT -> Ptr
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto operator*() USTL_NOEXCEPT -> RefMut
        {  return *pointer_;  }

        USTL_FORCEINLINE
        auto operator->() USTL_NOEXCEPT -> PtrMut
        {  return pointer_;  }
    
    protected:
        // It's all non-moveable and non-copyable constructors 
        // must be put under here. Because the first instance of 
        // Weak<T> is absolutely created via Rc<T>::degrade().
        template <typename, typename, typename>
        friend class Rc;

        USTL_FORCEINLINE
        Weak(PtrMut ptr, Counter *counter) USTL_NOEXCEPT
            : pointer_(ptr),
              counter_(counter)
        {
            if (counter_) {
                counter_->enable_weak();
            }
        }

        USTL_FORCEINLINE
        Weak(Rc<T, S> &rc) USTL_NOEXCEPT
            : pointer_(rc.pointer_),
              counter_(rc.counter_)
        {
            if (counter_) {
                counter_->inc_weak_ref();
            }
        }

    private:
        PtrMut   pointer_;
        Counter *counter_;
    };

    template <typename Derived, typename Base, typename A, typename S>
        requires traits::IsBaseOfV<RefCounter<Base, A, S>, Derived>
    USTL_FORCEINLINE
    auto make_rc(RefCounter<Base, A, S> *object)
        -> Rc<Derived, A, S> {
        /// There are still so many points need  we pay attention to:
        ///     1. Security of downcast without RTTI.
        return Rc<Derived, A, S>(
            static_cast<Derived *>(object), 
            reinterpret_cast<RefCounter<Derived, A, S> *>(object)
        );
    }

} // namespace ustl

#endif // #ifndef USTL_RC_HPP
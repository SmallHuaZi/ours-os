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
#include <ustl/traits/void.hpp>
#include <ustl/traits/enable_if.hpp>
#include <ustl/util/move.hpp>

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
        typedef T *         PtrMut;
        typedef T           Element;

        template <typename U>
        using SameCounter = RefCounter<U, A, S> ;

        typedef SameCounter<T>    Counter;
        typedef Weak<T,  A, S>    ThisWeak;

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

        Rc(Self const &other) 
            : pointer_(other.pointer_),
              counter_(other.counter_)
        {
            if (counter_) {
                counter_->inc_strong_ref();
            }
        }

        Rc(Self &&other) 
            : pointer_(other.pointer_),
              counter_(other.counter_)
        {
            other.counter_ = nullptr;
            other.pointer_ = nullptr;
        }

        /// From a raw object pointer to Rc<T>.
        /// In this case, we assert that the raw pointer is firstly into 
        /// Rc construction. 
        template <typename U>
            requires (traits::IsBaseOfV<U, T>)
        USTL_FORCEINLINE
        Rc(RefCounter<U, A, S> *other) USTL_NOEXCEPT
            : pointer_(static_cast<PtrMut>(other)),
              // First static_cast will help us to get correct pointer about inside counter.
              counter_(reinterpret_cast<Counter *>(static_cast<SameCounter<U> *>(other)))
        {
            if (counter_) {
                counter_->inc_strong_ref();
            }
        }

        /// Downcast/Upcast from a Rc<U> lvalue reference to Rc<T>
        template <typename U>
            requires (traits::IsBaseOfV<SameCounter<U>, T> || // T is derived from U
                      traits::IsBaseOfV<SameCounter<T>, U> ||
                      traits::IsSameV<U, T>)   // U is derived from T
        USTL_FORCEINLINE
        Rc(Rc<U, A, S> const &other) USTL_NOEXCEPT
            : pointer_(static_cast<PtrMut>(other.pointer_)),
              counter_(reinterpret_cast<Counter *>(other.counter_))
        {
            if (counter_) {
                counter_->inc_strong_ref();
            }
        }

        /// Downcast/Upcast from a Rc<U> lvalue reference to Rc<T>
        template <typename U>
            requires (traits::IsBaseOfV<SameCounter<U>, T> ||
                      traits::IsBaseOfV<SameCounter<T>, U> ||
                      traits::IsSameV<U, T>)
        USTL_FORCEINLINE
        Rc(Rc<U, A, S> &&other) USTL_NOEXCEPT
            : pointer_(static_cast<PtrMut>(other.pointer_)),
              counter_(reinterpret_cast<Counter *>(other.counter_))
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
            release();
        }

        /// Upcast
        USTL_FORCEINLINE
        auto operator=(Self const &other) USTL_NOEXCEPT -> Self & {
            release();
            pointer_ = other.pointer_;
            counter_ = other.counter_;
            if (counter_) {
                counter_->inc_strong_ref();
            }
            return *this;
        }

        /// Upcast
        USTL_FORCEINLINE
        auto operator=(Self &&other) USTL_NOEXCEPT -> Self & {
            release();
            pointer_ = other.pointer_;
            counter_ = other.counter_;
            other.counter_ = 0;
            other.pointer_ = 0;
            return *this;
        }

        /// Upcast
        template <typename U>
            requires (traits::IsBaseOfV<SameCounter<U>, T> || // T is derived from U and RefCouter<U>
                      traits::IsBaseOfV<SameCounter<T>, U> ||
                      traits::IsSameV<U, T>)   // U is derived from T
        USTL_FORCEINLINE
        auto operator=(Rc<U, A, S> const &other) USTL_NOEXCEPT -> Self & {
            release();
            pointer_ = static_cast<T *>(other.pointer_);
            counter_ = reinterpret_cast<Counter *>(static_cast<SameCounter<U> *>(other.counter_));
            if (counter_) {
                counter_->inc_strong_ref();
            }
            return *this;
        }

        /// Upcast
        template <typename U>
            requires (traits::IsBaseOfV<SameCounter<U>, T> || // T is derived from U
                      traits::IsBaseOfV<SameCounter<T>, U> ||
                      traits::IsSameV<U, T>)   // U is derived from T
        USTL_FORCEINLINE
        auto operator=(Rc<U, A, S> &&other) USTL_NOEXCEPT -> Self & {
            release();
            pointer_ = static_cast<T *>(other.pointer_);
            counter_ = reinterpret_cast<Counter *>(static_cast<SameCounter<U> *>(other.counter_));
            other.counter_ = 0;
            other.pointer_ = 0;
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
        auto as_ptr() USTL_NOEXCEPT -> T const * {
            return pointer_;
        }

        USTL_FORCEINLINE
        auto as_ptr_mut() USTL_NOEXCEPT -> T * {
            return pointer_;
        }

        template <typename U = T, traits::DisableIfT<traits::IsVoidV<U>, i32> = 0>
        USTL_FORCEINLINE
        auto as_ref_mut() USTL_NOEXCEPT -> U & {
            return *pointer_;
        }

        template <typename U = T, traits::DisableIfT<traits::IsVoidV<U>, i32> = 0>
        USTL_FORCEINLINE
        auto as_ref() USTL_NOEXCEPT -> U const & {
            return *pointer_;
        }

        template <typename U = T, traits::DisableIfT<traits::IsVoidV<U>, i32> = 0>
        USTL_FORCEINLINE
        auto operator*() const USTL_NOEXCEPT -> U const & {  
            return *pointer_;  
        }

        template <typename U = T, traits::DisableIfT<traits::IsVoidV<U>, i32> = 0>
        USTL_FORCEINLINE
        auto operator*() USTL_NOEXCEPT -> U &  {
            return *pointer_;
        }

        USTL_FORCEINLINE
        auto operator->() const USTL_NOEXCEPT -> Ptr
        {  return pointer_;  }

        USTL_FORCEINLINE
        auto operator->() USTL_NOEXCEPT -> PtrMut
        {  return pointer_;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        operator bool() USTL_NOEXCEPT
        {  return pointer_ != nullptr;  }

        USTL_FORCEINLINE USTL_CONSTEXPR
        friend auto operator ==(Self const &x, Self const &y) USTL_NOEXCEPT -> bool {
            return x.pointer_ == y.pointer_;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        friend auto operator ==(Self const &x, Ptr y) USTL_NOEXCEPT -> bool {
            return x.pointer_ == y;
        }

        USTL_FORCEINLINE USTL_CONSTEXPR
        auto release() -> void {
            if (!counter_) {
                return;
            }

            if (counter_->strong_count() == 1) {
                ThisWeak weak(pointer_, counter_);
                // counter_->disposer()(pointer_);
            } else {
                counter_->dec_strong_ref();
            }

            counter_ = 0;
            pointer_ = 0;
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

        Weak() = default;

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
        ~Weak() USTL_NOEXCEPT {
            if (counter_) {
                if (!counter_->strong_count() && counter_->weak_count() == 1) {
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
    auto downcast(Rc<Base, A, S> const &object) -> Rc<Derived, A, S> {
        return Rc<Derived, A, S>(object);
    }

    template <typename Derived, typename Base, typename A, typename S>
        requires traits::IsBaseOfV<RefCounter<Base, A, S>, Derived>
    USTL_FORCEINLINE
    auto downcast(Rc<Base, A, S> &&object) -> Rc<Derived, A, S> {
        return Rc<Derived, A, S>(ustl::move(object));
    }

    template <typename Derived, typename Base, typename A, typename S>
        requires (traits::IsBaseOfV<RefCounter<Base, A, S>, Derived> &&
                  traits::IsBaseOfV<Base, Derived>)
    USTL_FORCEINLINE
    auto make_rc(RefCounter<Base, A, S> *object) -> Rc<Derived, A, S> {
        return Rc<Derived, A, S>(static_cast<Base *>(object));
    }

    template <typename Base, typename Derived, typename A, typename S>
        requires traits::IsBaseOfV<RefCounter<Base, A, S>, Derived>
    USTL_FORCEINLINE
    auto make_rc(Rc<Derived, A, S> const &object) -> Rc<Base, A, S> {
        return Rc<Base, A, S>(object);
    }

    template <typename Base, typename Derived, typename A, typename S>
        requires traits::IsBaseOfV<RefCounter<Base, A, S>, Derived>
    USTL_FORCEINLINE
    auto make_rc(Rc<Derived, A, S> &&object) -> Rc<Base, A, S> {
        return Rc<Base, A, S>(ustl::move(object));
    }

} // namespace ustl

#endif // #ifndef USTL_RC_HPP
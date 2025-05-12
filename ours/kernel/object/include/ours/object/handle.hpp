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
#ifndef OURS_OBJECT_HANDLE_HPP
#define OURS_OBJECT_HANDLE_HPP 1

#include <ours/types.hpp>
#include <ours/object/dispatcher.hpp>

#include <ustl/rc.hpp>

namespace ours::object {
    class Dispatcher;

    template <typename KernelObject>
    class KernelHandle {
        static_assert(ustl::traits::IsBaseOfV<Dispatcher, KernelObject>, "");
        typedef KernelObject   Object;
      public:
        FORCE_INLINE
        KernelHandle(Object *object)
            : dispatcher_(object) { 
            DEBUG_ASSERT(object);
        }

        FORCE_INLINE
        KernelHandle(ustl::Rc<Object> object)
            : dispatcher_(ustl::move(object)) { 
            DEBUG_ASSERT(object); 
        }

        FORCE_INLINE
        KernelHandle(KernelHandle &&handle)
            : dispatcher_(ustl::move(handle)) 
        {}

        FORCE_INLINE
        auto dispatcher() const -> ustl::Rc<Object> const & {
            return dispatcher_;
        }

        template <typename T>
        FORCE_INLINE
        auto reset(ustl::Rc<T> dispatcher) -> void {
            static_assert(ustl::traits::IsBaseOfV<Dispatcher, T>);
            if (dispatcher_) {
                dispatcher_->on_zero();
            }

            dispatcher_ = ustl::move(dispatcher);
        }

        USTL_NOCOPYABLE(KernelHandle);
      private:
        ustl::Rc<Object> dispatcher_;
    };

    template <typename KernelObject>
    KernelHandle(KernelObject *) -> KernelHandle<KernelObject>;

    template <typename KernelObject>
    KernelHandle(ustl::Rc<KernelObject>) -> KernelHandle<KernelObject>;

} // namespace ours::object

#endif // #ifndef OURS_OBJECT_HANDLE_HPP
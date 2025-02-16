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

#ifndef OURS_MEM_NEW_HPP
#define OURS_MEM_NEW_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marker.hpp>

#include <ours/mem/gaf.hpp>

namespace ours {
    /// `Scope<T>` is a strong type allocator designed for managing the lifetime of objects.
    /// If an object is marked as `KernelObject` or `UserObject` by the macro `OURS_IMPL_MARKER_FOR`,
    /// the instance of it will be allocated by customized methods, namely the specialization of 
    /// `Scope<T>`.
    ///
    /// This allocator ensures type safety and proper resource management for both kernel and 
    /// user objects.
    ///
    /// Using \c `Scope<T>` through the following way:
    ///     struct Foo { int x, y, z; };
    ///     OURS_IMPL_MARKER_FOR(Foo, ours::marker::KernelObject);
    ///     auto ptr = new Scope<Foo>();
    ///
    ///     struct Bar { int x, y, z; };
    ///     OURS_IMPL_MARKER_FOR(Bar, ours::marker::UserObject);
    ///     auto ptr = new Scope<Bar>();
    ///
    template <typename T, typename = void>
    struct Scope;

    template <typename T>
        requires marker::HasImplementedV<T, marker::KernelObject>
    struct Scope<T>
    {
        CXX23_STATIC
        auto operator new(usize n, usize align = alignof(T)) -> void *
        {  return operator new(n, mem::GAF_KERNEL, align);  }

        CXX23_STATIC
        auto operator new(usize n, mem::Gaf gaf, usize align = alignof(T)) -> void *;

        CXX23_STATIC
        auto operator delete(void *ptr) -> void;

        auto raw() -> T *
        {  return static_cast<T *>(this);  }
    };

    template <typename T>
        requires marker::HasImplementedV<T, marker::UserObject>
    struct Scope<T>
    {
        CXX23_STATIC
        auto operator new(usize n, usize alignment = alignof(usize)) -> void *;

        CXX23_STATIC
        auto operator delete(void *ptr) -> void;

        auto raw() -> T *
        {  return static_cast<T *>(this);  }
    };

} // namespace ours

#endif // #ifndef OURS_MEM_NEW_HPP
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
#ifndef OURS_MEM_SCOPE_HPP
#define OURS_MEM_SCOPE_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/marker.hpp>

#include <ours/mem/gaf.hpp>
#include <ours/mem/types.hpp>

#include <arch/cache.hpp>

namespace ours {
    CXX11_CONSTEXPR
    static auto const kPageAlign = AlignVal(PAGE_SIZE);

    CXX11_CONSTEXPR
    static auto const kMaxPageAlign = AlignVal(MAX_PAGE_SIZE);

    CXX11_CONSTEXPR
    static auto const kCacheAlign = AlignVal(arch::kCacheSize);

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
    struct Scope<T> {
        FORCE_INLINE
        static auto operator new(usize n, AlignVal align = AlignVal(alignof(T))) -> void * {
            return operator new(n, mem::kGafKernel, align);
        }

        FORCE_INLINE
        static auto operator new(usize n, AlignVal align = AlignVal(alignof(T)), NodeId nid = MAX_NODE) -> void * {
            return operator new(n, mem::kGafKernel, align, nid);
        }

        static auto operator new(usize n, mem::Gaf gaf, AlignVal align, NodeId) -> void * {
            return nullptr;
        }

        FORCE_INLINE
        static auto operator new[](usize n, AlignVal align = AlignVal(alignof(T))) -> void * {
            return operator new(n, mem::kGafKernel, align);
        }

        FORCE_INLINE CXX23_STATIC
        auto operator new[](usize n, AlignVal align = AlignVal(alignof(T)), NodeId nid = MAX_NODE) -> void * {
            return operator new(n, mem::kGafKernel, align, nid);
        }

        static auto operator new[](usize n, mem::Gaf gaf, AlignVal align, NodeId) -> void * {
            return nullptr;
        }

        static auto operator delete(void *ptr) -> void;
        static auto operator delete[](void *ptr, usize count) -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        operator T*() {
            return raw();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto raw() -> T * {
            return reinterpret_cast<T *>(this);
        }
    };

    template <typename T>
        requires marker::HasImplementedV<T, marker::UserObject>
    struct Scope<T> {
        CXX23_STATIC
        auto operator new(usize n, usize alignment = alignof(usize)) -> void *;

        CXX23_STATIC
        auto operator delete(void *ptr) -> void;

        auto raw() -> T * {
            return static_cast<T *>(this);
        }
    };

} // namespace ours

#endif // #ifndef OURS_MEM_SCOPE_HPP
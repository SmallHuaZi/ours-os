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
#ifndef HEAP_NEW_HPP
#define HEAP_NEW_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/mem/gaf.hpp>

#include <ktl/object_cache.hpp>

auto operator new(ours::usize size, ours::usize align, ours::mem::Gaf gaf) CXX11_NOEXCEPT -> void *;

FORCE_INLINE
auto operator new(ours::usize size, ours::usize align) CXX11_NOEXCEPT -> void *
{  return operator new(size, align, ours::mem::kGafKernel);  }

auto operator delete(void *ptr) CXX11_NOEXCEPT -> void;

auto operator delete[](void *ptr) CXX11_NOEXCEPT -> void;

template <typename T, typename... Options>
auto operator new(ours::usize size, ours::usize align, ktl::ObjectCache<T, Options...> *cache) CXX11_NOEXCEPT -> void *
{}

template <typename T, typename... Options>
auto operator delete(void *ptr, ktl::ObjectCache<T, Options...> *cache) CXX11_NOEXCEPT -> void
{}

template <typename T, typename... Options>
auto operator delete(void *ptr, ours::usize n, ktl::ObjectCache<T, Options...> *cache) CXX11_NOEXCEPT -> void
{}

#endif // #ifndef HEAP_NEW_HPP
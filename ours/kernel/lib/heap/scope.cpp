#include "ktl/object_cache.hpp"
#include <heap/scope.hpp>

auto operator new(ours::usize size) -> void *
{  return operator new(size, alignof(void *), ours::mem::GAF_KERNEL);  }

auto operator new(ours::usize size, ours::usize align, ours::mem::Gaf gaf) CXX11_NOEXCEPT -> void *
{ return nullptr; }

auto operator delete(void *ptr) CXX11_NOEXCEPT -> void
{}

auto operator delete[](void *ptr) CXX11_NOEXCEPT -> void
{}
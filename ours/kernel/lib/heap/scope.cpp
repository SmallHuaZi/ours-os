#include <heap/scope.hpp>
#include <ktl/object_cache.hpp>
#include <ours/mem/mod.hpp>

struct PageAllocator;

template <int NumBytes>
using ObjectCache = ktl::ObjectCache<u8, PageAllocator, NumBytes>;

auto operator new(usize size, usize align, ours::mem::Gaf gaf) CXX11_NOEXCEPT -> void *
{ ours::panic(__func__); }

auto operator delete(void *ptr) CXX11_NOEXCEPT -> void
{}

auto operator delete[](void *ptr) CXX11_NOEXCEPT -> void
{}
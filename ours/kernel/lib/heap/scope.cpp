#include <heap/scope.hpp>
#include <ktl/object_cache.hpp>
#include <ours/mem/mod.hpp>

struct PageAllocator;

auto operator new(usize size, usize align, ours::mem::Gaf gaf) CXX11_NOEXCEPT -> void *
{ ours::panic(__func__); }
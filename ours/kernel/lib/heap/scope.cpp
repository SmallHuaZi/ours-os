#include <heap/scope.hpp>

auto operator new(ours::usize size, ours::usize align) -> void *
{ return nullptr; }

auto operator delete(void *ptr, ours::usize n) -> void
{}
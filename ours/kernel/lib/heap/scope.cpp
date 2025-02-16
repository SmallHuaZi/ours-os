#include <heap/scope.hpp>

auto operator new(ours::usize size, ours::usize align) -> void *
{}

auto operator delete(void *ptr, ours::usize n) -> void
{}
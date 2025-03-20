#include <heap/scope.hpp>
#include <ktl/object_cache.hpp>
#include <ours/mem/mod.hpp>

struct PageAllocator;

template <int NumBytes>
using ObjectCache = ktl::ObjectCache<u8, PageAllocator, NumBytes>;
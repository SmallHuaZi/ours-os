#include <ktl/new.hpp>

#include <ours/mem/object-cache.hpp>

using ours::mem::ObjectCache;

auto operator new(usize size, ktl::Gaf gaf) -> void * {
    return ktl::kmalloc(size, gaf);
}

auto operator new[](usize size, ktl::Gaf gaf) -> void * {
    return ktl::kmalloc(size, gaf);
}

auto operator new(usize size, ktl::Gaf gaf, ktl::NodeId nid) -> void * {
    return ktl::kmalloc(size, gaf, nid);
}

auto operator new[](usize size, ktl::Gaf gaf, ktl::NodeId nid) -> void * {
    return ktl::kmalloc(size, gaf, nid);
}

auto operator new(usize size, ObjectCache &cache, ktl::Gaf gaf) -> void * {
    return cache.allocate(gaf, ours::mem::current_node());
}

auto operator new[](usize size, ObjectCache &cache, ktl::Gaf gaf) -> void * {
    return cache.allocate(size / cache.object_size(), gaf);
}

auto operator new(usize size, ObjectCache &cache, ktl::Gaf gaf, ktl::NodeId nid) -> void * {
    return cache.allocate(gaf, nid);
}

auto operator new[](usize size, ObjectCache &cache, ktl::Gaf gaf, ktl::NodeId nid) -> void * {
    return cache.allocate(size / cache.object_size(), gaf, nid);
}

auto operator delete(void *ptr) -> void {
    return ktl::kfree(ptr);
}

auto operator delete[](void *ptr) -> void {
    return ktl::kfree(ptr);
}
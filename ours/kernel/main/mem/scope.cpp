#include <ours/mem/scope.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/assert.hpp>
#include <ours/mem/object-cache.hpp>

#include <ours/platform/init.hpp>

#include <ustl/bit.hpp>

namespace ours::mem {
    CXX11_CONSTEXPR
    static auto const kNumMallocCaches = 10;

    CXX11_CONSTEXPR
    static auto const kMallocMinOrder = 3;

    CXX11_CONSTEXPR
    static auto const kMinMallocSize= sizeof(usize);

    CXX11_CONSTEXPR
    static auto const kMaxMallocSize= BIT(kNumMallocCaches + kMallocMinOrder);

    static auto get_allocation_index(usize size) -> usize {
        if (!size) {
            return 0;
        }
    
        if (size <= kMaxMallocSize) {
            return ustl::bit_width<usize>(size - 1) - kMallocMinOrder;
        }

        panic("Unreachable");
    }

    static ustl::Array<ObjectCache, kNumMallocCaches>  s_kmalloc_cache;

    auto kmalloc(usize size, Gaf gaf, NodeId nid) -> void * {
        if (size > kMaxMallocSize) {
            // Now we do not support to allocate frames from kmalloc
            return nullptr;
        }
        if (nid == MAX_NODE) {
            nid = current_node();
        }

        auto const index = get_allocation_index(size);
        return s_kmalloc_cache[index].do_allocate(gaf, nid);
    }

    auto kmalloc(usize size, Gaf gaf) -> void * {
        if (size > kMaxMallocSize) {
            // Now we do not support to allocate frames from kmalloc
            return nullptr;
        }

        auto const index = get_allocation_index(size);
        return s_kmalloc_cache[index].do_allocate(gaf, current_node());
    }

    auto kfree(void *object) -> void {
        auto slab = role_cast<PfRole::Slab>(virt_to_folio(object));
        DEBUG_ASSERT(slab && slab->object_cache);
        slab->object_cache->do_deallocate(slab, object);
    }

    INIT_CODE
    static auto init_kmalloc() -> void {
        char name[16];
        for (auto i = 0; i < s_kmalloc_cache.size(); ++i) {
            auto status = s_kmalloc_cache[i].init(name, BIT(i + kMallocMinOrder), alignof(usize), {});
            if (Status::Ok != status) {
                panic("Failed to initialize kmalloc caches");
            }
        }
    }
    /// It is still not very stable. Let's try initializaing it this way first.
    GKTL_INIT_HOOK(KmallocInit, init_kmalloc, PlatformInitLevel::Pmm);

} // namespace ours::mem

/// The following standard operator new/delete was temporarily placed.
auto operator new(usize size) -> void * {
    return ours::mem::kmalloc(size, ours::mem::kGafKernel, ours::mem::current_node());
}

auto operator new(usize size, std::align_val_t align) -> void * {
    return ours::mem::kmalloc(size, ours::mem::kGafKernel, ours::mem::current_node());
}

auto operator new[](usize size) -> void * {
    return ours::mem::kmalloc(size, ours::mem::kGafKernel, ours::mem::current_node());
}
auto operator new[](usize size, std::align_val_t align) -> void * {
    return ours::mem::kmalloc(size, ours::mem::kGafKernel, ours::mem::current_node());
}

auto operator delete(void *ptr) -> void {
    return ours::mem::kfree(ptr);
}

auto operator delete[](void *ptr) -> void {
    return ours::mem::kfree(ptr);
}
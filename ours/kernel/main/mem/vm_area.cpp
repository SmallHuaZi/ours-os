#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/object-cache.hpp>

#include <gktl/init_hook.hpp>

namespace ours::mem {
    VmAreaHandler g_normal_vma_handler;
    VmAreaHandler g_mapping_vma_handler;

    CXX11_CONSTEXPR
    static VmaFlags const kVmaFlagsInitAllowed = VmaFlags::Mergeable | 
                                                 VmaFlags::Anonymous |
                                                 VmaFlags::Normal;

    VmArea::VmArea(ustl::Rc<VmAspace> aspace, 
                   VirtAddr base, 
                   usize size, 
                   MmuFlags rights, 
                   VmaFlags flags, 
                   char const *name)
        : Base(),
          name_(name),
          flags_((flags & kVmaFlagsInitAllowed)),
          base_(base),
          size_(size),
          aspace_(aspace),
          rights_(rights),
          handler_(),
          set_hook_(),
          list_hook_()
    {}

    auto VmArea::activate() -> void {
        this->flags_ |= VmaFlags::Active;
        this->handler_ = std::addressof(g_normal_vma_handler);
    }

    static ustl::Rc<ObjectCache> s_vma_cache;

    auto VmArea::create(ustl::Rc<VmAspace> aspace, VirtAddr base, usize size, 
                        MmuFlags rights, VmaFlags vmaf, char const *name) 
        -> ustl::Result<ustl::Rc<Self>, Status> {
        DEBUG_ASSERT(aspace, "Given a invalid aspace");
        auto self = s_vma_cache->allocate<Self>(aspace, base, size, rights, vmaf, name);
        if (!self) {
            return ustl::err(Status::OutOfMem);
        }

        return ustl::ok(ustl::make_rc<VmArea>(self));
    }

    INIT_CODE
    static auto init_vma_cache() -> void {
        s_vma_cache = make_object_cache<VmArea>("vma-cache", OcFlags::Folio);
        if (!s_vma_cache) {
            panic("Failed to create object cache for VmArea");
        }
    }
    GKTL_INIT_HOOK(VmCacheInit, init_vma_cache, gktl::InitLevel::PlatformEarly);
}
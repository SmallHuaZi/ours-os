#include "ours/config/lang_items.hpp"
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <memory>

namespace ours::mem {
    VmAreaHandler G_NORMAL_VMA_HANDLER;
    VmAreaHandler G_MAPPING_VMA_HANDLER;

    CXX11_CONSTEXPR
    static VmaFlags const VMAF_INIT_ALLOWED = VmaFlags::Mergeable | 
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
          flags_((flags & VMAF_INIT_ALLOWED)),
          base_(base),
          size_(size),
          aspace_(aspace),
          rights_(rights),
          handler_(),
          set_hook_(),
          list_hook_()
    {}

    auto VmArea::activate() -> void
    {
        this->flags_ |= VmaFlags::Active;
        this->handler_ = std::addressof(G_NORMAL_VMA_HANDLER);
    }

}
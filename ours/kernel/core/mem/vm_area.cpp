#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <memory>

namespace ours::mem {
    VmAreaHandler G_NORMAL_VMA_HANDLER;
    VmAreaHandler G_MAPPING_VMA_HANDLER;

    VmArea::VmArea(ustl::Rc<VmAspace> aspace, gktl::Range<VirtAddr> range, VmAreaRight right)
        : flags_(VmaFlags::Normal | VmaFlags::Inactive),
          range_(range),
          aspace_(aspace),
          rights_(right),
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
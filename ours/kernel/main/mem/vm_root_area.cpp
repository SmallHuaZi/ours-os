#include <ours/mem/vm_root_area.hpp>

namespace ours::mem {

    auto VmRootArea::create_subvma(VirtAddr base, usize size, usize align, MmuFlags flags, ustl::Rc<VmObject> *)
        -> ustl::Rc<VmArea>
    {
        if (auto vma = create_subvma_inner(base, size, align, flags)) {
            vma->activate();
        }

        return nullptr;
    }

    auto VmRootArea::destory_subvma(VirtAddr base, usize size) -> Status 
    {  return Status::Unimplemented;  }

    auto VmRootArea::reserve_subvma(char const *name, VirtAddr base, usize size, MmuFlags flags) -> Status
    {  return Status::Unimplemented;  }

    auto VmRootArea::find_subvma(VirtAddr virt_addr) -> ustl::Rc<VmArea> 
    {
        CXX11_CONSTEXPR 
        auto const comp = [] (VirtAddr x, VmArea const &y)
        {  return x < y.base_;  };

        auto result = subvma_set_.upper_bound(virt_addr, comp);
        if (result == subvma_set_.end()) {
            return nullptr;
        }

        return std::addressof(*result);
    }

    auto VmRootArea::create_subvma_inner(VirtAddr base, usize size, usize align, MmuFlags flags)
        -> ustl::Rc<VmArea>
    {
        // if (base < base_|| range_.end < range.end) {
            // println("range {} is invalid.", range);
            // return nullptr;
        // }

        // range is legal, but maybe it has existed.
        return nullptr;
    }
} // namespace ours::mem
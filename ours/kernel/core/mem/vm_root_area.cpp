#include <ours/mem/vm_root_area.hpp>

namespace ours::mem {

    auto VmRootArea::create_subvma(gktl::Range<VirtAddr> range, usize align, VmaFlags flags)
        -> ustl::Rc<VmArea>
    {
        return create_subvma_inner(range, align, flags);
    }

    auto VmRootArea::create_subvma(gktl::Range<VirtAddr> range, 
                                   usize align, 
                                   VmaFlags flags,
                                   ustl::Rc<VmObject> *object)
        -> ustl::Rc<VmArea>
    {
        if (auto vma = create_subvma_inner(range, align, flags)) {
            vma->activate();
        }

        return nullptr;
    }

    auto VmRootArea::destory_subvma(gktl::Range<VirtAddr> range) -> Status 
    {  return Status::Fail;  }

    auto VmRootArea::find_subvma(VirtAddr virt_addr) -> ustl::Rc<VmArea> 
    {
        CXX11_CONSTEXPR 
        auto const comp = [] (VirtAddr x, VmArea const &y)
        {  return x < y.range_.start;  };

        auto result = subvma_set_.upper_bound(virt_addr, comp);
        if (result == subvma_set_.end()) {
            return nullptr;
        }

        return std::addressof(*result);
    }

    auto VmRootArea::create_subvma_inner(gktl::Range<VirtAddr> range, usize align, VmaFlags flags)
        -> ustl::Rc<VmArea>
    {
        if (range.start < range_.start || range_.end < range.end) {
            // println("range {} is invalid.", range);
            return nullptr;
        }

        // range is legal, but maybe it has existed.
        return 0;
    }
} // namespace ours::mem
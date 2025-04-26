#include <ours/mem/vm_area_or_mapping.hpp>
#include <ustl/mem/align.hpp>

namespace ours::mem {
    VmAreaOrMapping::VmAreaOrMapping(VirtAddr base, usize size, VmaFlags vmaf, 
                                     VmArea *parent, VmAspace *aspace, char const *name)
        : base_(base), size_(size), vmaf_(vmaf), parent_(parent), aspace_(aspace), 
          name_(name), children_hook_()
    {}

    FORCE_INLINE
    auto VmAreaOrMapping::validate_mmuflags(MmuFlags mmuf) const -> bool {
        VmaFlags vmaf{};
        if (!!(mmuf & MmuFlags::Readable)) {
            vmaf |= VmaFlags::Read;
        }
        if (!!(mmuf & MmuFlags::Writable)) {
            vmaf |= VmaFlags::Write;
        }
        if (!!(mmuf & MmuFlags::Executable)) {
            vmaf |= VmaFlags::Exec;
        }

        auto perm_allowed = (vmaf_ & VmaFlags::PermMask);
        if (((vmaf & VmaFlags::PermMask) | perm_allowed) != perm_allowed) {
            return false;
        }

        // Except the requirements above, a qualifying MMU flag has at least one
        // performing permission yet.
        return !!(mmuf & MmuFlags::PermMask);
    }

    auto VmaSet::has_range(VirtAddr base, usize size) const -> bool {
        auto prev = upper_bound(base, [] (VirtAddr base, Element const &x) {
            return base < x.base_;
        });

        if (prev == end()) {
            return false;
        }
        auto next = prev--;
        if (next->base_ < base + size && prev->base_ + prev->size_ < base) {
            return false;
        }

        return true;
    }

    auto VmaSet::find_spot(usize size, AlignVal align, VirtAddr lower_bound, VirtAddr upper_bound)
        const -> ustl::Result<VirtAddr, Status> {
        if (!size) {
            return ustl::err(Status::InvalidArguments);
        }
        if (align < PAGE_SIZE) {
            align = PAGE_SIZE;
        }

        auto request_size = ustl::mem::align_up(size, align);
        VirtAddr addr = ustl::mem::align_up(lower_bound, PAGE_SIZE);
        for (auto i = begin(), last = end(); i != last; ++i) {
            if (addr > upper_bound) {
                return ustl::err(Status::NotFound);
            }
            if (i->base_ > addr + request_size) {
                return ustl::ok(addr);
            }

            addr = ustl::mem::align_up(i->base_ + i->size_, align);
        }

        if (addr + request_size < upper_bound) {
            return ustl::ok(addr);
        }
        return ustl::err(Status::NotFound);
    }

} // namespace ours::mem
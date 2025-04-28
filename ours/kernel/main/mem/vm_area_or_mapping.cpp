#include <ours/mem/vm_area_or_mapping.hpp>
#include <ours/mem/vm_area.hpp>

#include <ustl/mem/align.hpp>
#include <logz4/log.hpp>

namespace ours::mem {
    VmAreaOrMapping::VmAreaOrMapping(VirtAddr base, usize size, VmaFlags vmaf, 
                                     VmArea *parent, VmAspace *aspace, char const *name)
        : canary_(), base_(base), size_(size), vmaf_(vmaf), parent_(parent), 
          aspace_(aspace), name_(name), children_hook_()
    {}

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

    auto VmAreaOrMapping::activate() -> void {
        mark_active();
        parent_->subvmas_.insert(*this);
    }

    auto VmAreaOrMapping::destroy() -> void {
        vmaf_ &= ~VmaFlags::Area;
        auto to_erase = parent_->subvmas_.iterator_to(*this);
        parent_->subvmas_.erase(to_erase);
    }

    auto VmaSet::has_range(VirtAddr base, usize size) const -> bool {
        auto iter = upper_bound(base + size);
        if (iter != end()) {
            if (iter->base_ <= base + size - 1) {
                return true;
            }
        }

        --iter;
        if (iter != end()) {
            if (iter->base_ + iter->size_ - 1 >= base) {
                return true;
            }
        }

        return false;
    }

    auto VmaSet::find_spot(usize size, AlignVal align, VirtAddr low_limit, VirtAddr high_limit)
        const -> ustl::Result<VirtAddr, Status> {
        if (!size) {
            return ustl::err(Status::InvalidArguments);
        }
        if (align < PAGE_SIZE) {
            align = PAGE_SIZE;
        }

        auto request_size = ustl::mem::align_up(size, align);
        VirtAddr addr = ustl::mem::align_up(low_limit, PAGE_SIZE);
        for (auto i = lower_bound(addr), last = end(); i != last; ++i) {
            if (addr > high_limit) {
                return ustl::err(Status::NotFound);
            }
            if (i->base_ >= addr + request_size) {
                return ustl::ok(addr);
            }

            addr = ustl::mem::align_up(i->base_ + i->size_, align);
        }

        if (addr + request_size <= high_limit) {
            return ustl::ok(addr);
        }
        return ustl::err(Status::NotFound);
    }

} // namespace ours::mem
#include <ours/arch/vm_aspace.hpp>

namespace ours::mem {
    ArchVmAspace::ArchVmAspace(VirtAddr base, usize size, VmasFlags flags)
        : range_(base, size),
          flags_(flags)
    {}

    auto ArchVmAspace::init() -> Status
    {
        if (bool(VmasFlags::Guest & flags_)) {
            auto ept = new (&page_table_storaged_) X86PageTableEpt();
            auto status = ept->init();
            if (status != Status::Ok) {
                return status;
            }
            page_table_ = ept;
        } else if (bool(VmasFlags::Kernel & flags_)) [[unlikely]] {
            auto mmu = new (&page_table_storaged_) X86PageTableMmu();
            auto status = mmu->init_kernel();
            if (status != Status::Ok) {
                return status;
            }
            page_table_ = mmu;
        } else [[likely]] {
            auto mmu = new (&page_table_storaged_) X86PageTableMmu();
            auto status = mmu->init();
            if (status != Status::Ok) {
                return status;
            }
            page_table_ = mmu;
        }

        return Status::Ok;
    }
}
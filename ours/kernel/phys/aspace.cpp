#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ustl/algorithms/generation.hpp>
#include <ustl/function/bind.hpp>

namespace ours::phys {
    struct PhysToTable {
        auto operator()(PhysAddr pa_table) -> VirtAddr {
            return VirtAddr(pa_table);
        }
    };

    auto Aspace::map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ktl::Result<usize> {
        DEBUG_ASSERT(allocation_lower_bound_ < allocation_upper_bound_, "");
        auto allocator = ustl::function::bind_front(&Aspace::alloc_page_table, this);

        if CXX17_CONSTEXPR (kDualAspace) {
            UpperPaging::map(upper_pgd_, PhysToTable(), allocator, va, pa, n, flags);
        }
        LowerPaging::map(lower_pgd_, PhysToTable(), allocator, va, pa, n, flags);

        return ustl::ok(n);
    }

    auto Aspace::unmap(VirtAddr va, usize n) -> Status {
    }

    auto Aspace::alloc_page_table(usize size, usize alignment) const -> PhysAddr {
        PhysAddr phys;
        if (allocation_lower_bound_ < allocation_upper_bound_) {
            phys = global_bootmem()->allocate(size, alignment, allocation_lower_bound_, allocation_upper_bound_);
        } else {
            phys = global_bootmem()->allocate(size, alignment);
        }

        if (phys) {
            ustl::algorithms::fill_n(reinterpret_cast<usize *>(phys), size / sizeof(usize), 0);
        }
        return phys;
    }

    auto create_identity_map_for_ram(Aspace &aspace, PhysAddr max_limit) -> void {
        CXX11_CONSTEXPR 
        auto const mmuf = mem::MmuFlags::PermMask | mem::MmuFlags::Present;

        auto const mem = global_bootmem();
        BootMem::IterationContext context(bootmem::RegionType::Normal);
        while (auto region = mem->iterate(context)) {
            if (region->base >= max_limit) {
                break;
            }

            auto result = aspace.map_identically(region->base, region->size / PAGE_SIZE, mmuf);
            if (result.is_err()) {
                panic("Identically map {} pages", result.unwrap());
            }
        }
    }

} // namespace ours::phys
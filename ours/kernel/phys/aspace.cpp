#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ustl/algorithms/generation.hpp>
#include <ustl/function/bind.hpp>
#include <ours/arch/aspace_layout.hpp>

namespace ours::phys {
    struct PhysToTable {
        auto operator()(PhysAddr pa_table) -> VirtAddr {
            return VirtAddr(pa_table);
        }
    };

    auto Aspace::map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ustl::Result<void, MapError> {
        DEBUG_ASSERT(allocation_lower_bound_ <= allocation_upper_bound_, "");
        auto allocator = ustl::function::bind_front(&Aspace::alloc_page_table, this);

        if CXX17_CONSTEXPR (kDualAspace) {
            UpperPaging::map(upper_pgd_, PhysToTable(), allocator, va, pa, n, flags);
        }
        
        auto result = LowerPaging::map(lower_pgd_, PhysToTable(), allocator, va, pa, n, flags);
        if (!result) {
            return ustl::err(result.unwrap_err());
        }
        DEBUG_ASSERT(result, "Failed to map");

        return ustl::ok();
    }

    auto Aspace::unmap(VirtAddr va, usize n) -> Status {
        return Status::Unimplemented;
    }

    auto Aspace::alloc_page_table(usize size, usize alignment) const -> PhysAddr {
        PhysAddr phys;
        if (allocation_lower_bound_ < allocation_upper_bound_) {
            phys = global_bootmem()->allocate_bounded(size, alignment, allocation_lower_bound_, allocation_upper_bound_);
        } else {
            phys = global_bootmem()->allocate(size, alignment);
        }

        if (phys) {
            ustl::algorithms::fill_n(reinterpret_cast<usize *>(phys), size / sizeof(usize), 0);
        }

        dprintln("Allocate page table (p:0x{:X}, s:0x{:X})", phys, size);
        return phys;
    }

    auto create_identity_map_for_ram(Aspace &aspace, PhysAddr max_limit) -> void {
        auto const map = [&] (PhysAddr base, PhysAddr end) {
            auto const size = end - base;
            dprintln("Map identically (0x{:X}, 0x{:X})", base, size);

            CXX11_CONSTEXPR 
            auto const mmuf = mem::MmuFlags::PermMask | mem::MmuFlags::Present;
            auto result = aspace.map_identically(base, size / PAGE_SIZE, mmuf);
            if (result.is_err()) {
                panic("Identically map {} pages", to_string(result.unwrap_err()));
            }
        };

        auto const mem = global_bootmem();

        PhysAddr const sentinel = ustl::NumericLimits<PhysAddr>::max();
        PhysAddr start = sentinel, end = sentinel;
        BootMem::IterationContext context(bootmem::RegionType::Normal);
        while (auto region = mem->iterate(context)) {
            if (region->base >= max_limit) {
                break;
            }

            if (start != sentinel && end != region->base) {
                map(start, end);
                start = sentinel;
                end = sentinel;
            }

            if (sentinel == start) {
                start = region->base;
            }

            if (sentinel == end || end == region->base) {
                end = region->end();
            }
        }

        if (start != sentinel && end != sentinel) {
            map(start, end);
        }
    }

} // namespace ours::phys
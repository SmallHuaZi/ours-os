#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ustl/algorithms/generation.hpp>
#include <ustl/function/bind.hpp>

namespace ours::phys {
    struct PhysToTable {
        typedef Aspace::LowerPaging::LevelType  LevelType; 

        template <LevelType Level>
        using Table = Aspace::LowerPaging::Table<Level>; 

        // BUG(SmallHuaZi): This code relies on the assumption that:
        // Page table entries at each level share the same layout. 
        // Regardless of the value held by `Level`, `Table<Level>` is 
        // treated as the same type. While invoking this method hasn't 
        // reported any errors so far, this assumption is flawed in its 
        // design. We need to address and correct it.
        template <LevelType Level>
        auto operator()(PhysAddr pa_table) -> Table<Level> & {
            return *reinterpret_cast<Table<Level> *>(pa_table);
        }
    };

    auto Aspace::map(VirtAddr va, usize n, PhysAddr pa, mem::MmuFlags flags) -> ktl::Result<usize> {
        DEBUG_ASSERT(allocation_lower_bound_ < allocation_upper_bound_, "");
        auto const allocator = ustl::function::bind(&Aspace::alloc_page_table, this);

        if CXX17_CONSTEXPR (kDualAspace) {
            UpperPaging::map(pgd_, PhysToTable(), allocator, va, n, pa, flags);
        }
        LowerPaging::map(pgd_, PhysToTable(), allocator, va, n, pa, flags);
    }

    auto Aspace::unmap(VirtAddr va, usize n) -> Status {
    }

    auto Aspace::alloc_page_table(usize size, usize alignment) const -> ustl::Option<PhysAddr> {
        DEBUG_ASSERT(global_bootmem(), "Never be NULL");
        if (auto phys = global_bootmem()->allocate_bounded(size, alignment, allocation_lower_bound_, allocation_upper_bound_)) {
            ustl::algorithms::fill_n((usize *)phys, size / sizeof(usize), 0);
            return phys;
        }
        return ustl::NONE;
    }

} // namespace ours::phys
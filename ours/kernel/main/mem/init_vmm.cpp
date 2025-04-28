#include <ours/mem/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_page.hpp>
#include <ours/mem/physmap.hpp>
#include <ours/mem/vm_object_paged.hpp>

#include <ours/assert.hpp>
#include <ours/init.hpp>
#include <ours/panic.hpp>
#include <ours/status.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/limits.hpp>
#include <ustl/lazy_init.hpp>
#include <ustl/views/span.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/algorithms/minmax.hpp>

#include <logz4/log.hpp>

#include <arch/cache.hpp>

namespace ours::mem {
    struct PhysVma {
        char const *name;
        VirtAddr base;
        usize size;
        VmaFlags vmaf;
    };

    CXX11_CONSTEXPR
    static MmuFlags const kPresetVmaMmuFlags = MmuFlags::PermMask;

    /// Used in init_vmm_postheap.
    /// TODO(SmallHuaZi) These be provided by PhysBoot through HandOff rather than
    /// direct to use linking symbols.
    INIT_CONST
    static PhysVma s_phys_vmos[] = {
        {
            .name = "k:code",
            .base = VirtAddr(kKernelCodeStart),
            .size = usize(kKernelCodeEnd - kKernelCodeStart),
            .vmaf = VmaFlags::Exec | VmaFlags::Read
        },
        {
            .name = "k:data",
            .base = VirtAddr(kKernelDataStart),
            .size = usize(kKernelDataEnd - kKernelDataStart),
            .vmaf = VmaFlags::Write | VmaFlags::Read
        },
        {
            .name = "k:rodata",
            .base = VirtAddr(kKernelRodataStart),
            .size = usize(kKernelRodataEnd - kKernelRodataStart),
            .vmaf = VmaFlags::Read
        },
        {
            .name = "k:bss",
            .base = VirtAddr(kKernelBssStart),
            .size = usize(kKernelBssEnd - kKernelBssStart),
            .vmaf = VmaFlags::Read | VmaFlags::Write
        },
        {
            .name = "k:init",
            .base = VirtAddr(kKernelInitStart),
            .size = usize(kKernelInitEnd - kKernelInitStart),
            .vmaf = VmaFlags::Read | VmaFlags::Write | VmaFlags::Exec
        },
        {
            .name = "k:physmap",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = PhysMap::kVirtBase,
            .size = PhysMap::kSize,
            .vmaf = VmaFlags::Read | VmaFlags::Write
        },
        {
            .name = "k:padding",
            .base = PhysMap::kVirtBase + PhysMap::kSize,
            // Reserve a max page to prevent over prevent overwring.
            .size = MAX_PAGE_SIZE,
            .vmaf = VmaFlags::Read
        },
    };

    INIT_CODE
    static auto entrol_preset_vma(PhysVma const &phys_vma) -> void {
        auto [base, end] = resolve_page_range(phys_vma.base, phys_vma.base + phys_vma.size);
        log::trace("Reserve area {}: [{:X}, {:X})", phys_vma.name, base, end);

        auto kaspace = VmAspace::kernel_aspace();
        auto root_vma = kaspace->root_vma();
        ustl::Rc<VmArea> vma;
        auto status = root_vma->create_subvma(phys_vma.base - root_vma->base(), phys_vma.size, phys_vma.vmaf, 
            phys_vma.name, VmMapOption::None, &vma);
        DEBUG_ASSERT(Status::Ok == status);

        status = vma->reserve(0, phys_vma.size, extract_permissions(phys_vma.vmaf), phys_vma.name);
        DEBUG_ASSERT(Status::Ok == status);
    }

    INIT_CODE
    static auto init_vmm_preheap() -> void {
        VmAspace::init_kernel_aspace();

        for (auto i = 0; i < std::size(s_phys_vmos); ++i) {
            entrol_preset_vma(s_phys_vmos[i]);
        }

        auto frame = alloc_frame(kGafKernel, usize(0));
        if (!frame) {
            panic("Failed to allocate the zero page");
        }
        g_zero_page = role_cast<PfRole::Vmm>(frame);
        g_zero_page->mark_pinned();

        VmAspace::kernel_aspace()->root_vma()->dump();
    }

    INIT_CODE
    static auto init_heap() -> void {

    }

    /// Requires:
    ///     1). 
    INIT_CODE 
    auto init_vmm() -> void {
        init_vmm_preheap();

        init_heap();
    }
}
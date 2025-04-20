#include <ours/mem/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_page.hpp>
#include <ours/mem/physmap.hpp>

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
    struct PresetVmaInfo {
        char const *name;
        VirtAddr base;
        usize size;
        MmuFlags rights;
        VmaFlags flags;
    };

    enum PresetVmas {
        KernelImageVma,
        PhysMapVma,
        VflatModelVma,
        PaddingVma,
        MaxNumPresetVmas,
    };

    CXX11_CONSTEXPR
    static VmaFlags const kPresetVmaFlags = VmaFlags::Mapped;

    CXX11_CONSTEXPR
    static MmuFlags const kPresetVmaMmuFlags = MmuFlags::PermMask;

    /// Used in init_vmm_postheap
    INIT_CONST
    static PresetVmaInfo const kPresetVmas[] = {
        {
            .name = "k:code",
            .base = VirtAddr(kKernelCodeStart),
            .size = usize(kKernelCodeEnd - kKernelCodeStart),
            .rights = MmuFlags::Executable | MmuFlags::Readable
        },
        {
            .name = "k:data",
            .base = VirtAddr(kKernelDataStart),
            .size = usize(kKernelDataEnd - kKernelDataStart),
            .rights = MmuFlags::Writable | MmuFlags::Readable
        },
        {
            .name = "k:rodata",
            .base = VirtAddr(kKernelRodataStart),
            .size = usize(kKernelRodataEnd - kKernelRodataStart),
            .rights = MmuFlags::Readable
        },
        {
            .name = "k:bss",
            .base = VirtAddr(kKernelBssStart),
            .size = usize(kKernelBssEnd - kKernelBssStart),
            .rights = MmuFlags::Readable
        },
        {
            .name = "k:init",
            .base = VirtAddr(kKernelInitStart),
            .size = usize(kKernelInitEnd - kKernelInitStart),
            .rights = MmuFlags::Readable | MmuFlags::Writable | MmuFlags::Executable
        },
        {
            .name = "k:physmap",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = PhysMap::kVirtBase,
            .size = PhysMap::kSize,
            .rights = kPresetVmaMmuFlags,
            .flags = VmaFlags::Mapped,
        },
        {
            .name = "k:padding",
            .base = PhysMap::kVirtBase + PhysMap::kSize,
            // Reserve a max page to prevent over prevent overwring.
            .size = MAX_PAGE_SIZE,
            .rights = kPresetVmaMmuFlags,
        },
    };

    INIT_CODE
    static auto init_vmm_preheap() -> void {
        VmAspace::init_kernel_aspace();
        auto kaspace = VmAspace::kernel_aspace();

        // Initialize all of named vmas.
        for (auto i = 0; i < MaxNumPresetVmas; ++i) {
            auto const &region = kPresetVmas[i];
            auto vma = VmArea::create(kaspace, region.base, region.size, region.rights, region.flags, region.name);
            if (!vma) {
                panic("[{}]: Failed to allocate vma for region[name: {} | base: 0x{:X} | size: 0x{:X}]",
                      to_string(vma.unwrap_err()), region.name, region.base, region.size);
            }
            (*vma)->activate();
        }

        auto frame = alloc_frame(kGafKernel, usize(0));
        if (!frame) {
            panic("Failed to allocate the zero page");
        }
        g_zero_page = role_cast<PfRole::Vmm>(frame);
        g_zero_page->mark_pinned();
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
#include <ours/mem/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/physmap.hpp>

#include <ours/assert.hpp>
#include <ours/init.hpp>
#include <ours/panic.hpp>
#include <ours/status.hpp>
#include <ours/cpu_local.hpp>

#include <ustl/limits.hpp>
#include <ustl/lazy_init.hpp>
#include <ustl/views/span.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/algorithms/minmax.hpp>

#include <logz4/log.hpp>

#include <arch/cache.hpp>

using ustl::algorithms::min;
using ustl::algorithms::max;
using ustl::algorithms::clamp;
using ustl::mem::construct_at;

namespace ours::mem {
    NO_MANGLE {
        PhysAddr KERNEL_PHYS_BASE;
        VirtAddr KERNEL_VIRT_BASE;
    }

    struct PresetVmaInfo
    { 
        char const *name;
        VirtAddr base;
        usize size;
        MmuFlags rights;
        VmaFlags flags;
        VmArea *altvma;
    };

    enum PresetVmas {
        KernelImageVma,
        PhysMapVma,
        VflatModelVma,
        PaddingVma,
        MaxNumPresetVmas,
    };
    static ustl::LazyInit<VmArea> PRESET_VMAS[MaxNumPresetVmas];

    CXX11_CONSTEXPR
    static VmaFlags const PRESET_VMAF = VmaFlags::Mapped;

    CXX11_CONSTEXPR
    static MmuFlags const PRESET_MMUF = MmuFlags::PermMask;

    INIT_DATA
    static PresetVmaInfo NAMED_PRESET_VMAS[] = {
        {
            .name = "k:image",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = VirtAddr(KERNEL_IMAGE_START),
            .size = usize(KERNEL_IMAGE_END - KERNEL_IMAGE_START),
            .rights = PRESET_MMUF,
            .flags = VmaFlags::Mapped,
            .altvma = PRESET_VMAS[KernelImageVma].data(),
        },
        {
            .name = "k:physmap",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = PhysMap::VIRT_BASE,
            .size = PhysMap::SIZE,
            .rights = PRESET_MMUF,
            .flags = VmaFlags::Mapped,
            .altvma = PRESET_VMAS[PhysMapVma].data(),
        },
        {
            .name = "k:padding",
            .base = PhysMap::VIRT_BASE + PhysMap::SIZE,
            // Reserve a max page to prevent over prevent overwring.
            .size = MAX_PAGE_SIZE,
            .rights = PRESET_MMUF,
            .altvma = PRESET_VMAS[PaddingVma].data(),
        },
    };

    INIT_CODE
    static auto init_vmm_preheap() -> void
    {
        VmAspace::init_kernel_aspace();
        auto kaspace = VmAspace::kernel_aspace();

        if constexpr (OURS_CONFIG_KASLR) {
            VirtAddr const link_load_addr = VirtAddr(KERNEL_IMAGE_START);
            VirtAddr const real_load_addr = get_kernel_virt_base();
            isize const delta = real_load_addr - link_load_addr;

            for (auto i = 0; i < MaxNumPresetVmas; ++i) {
                NAMED_PRESET_VMAS[i].base += delta;
            }
        }

        // Initialize all of named vmas.
        for (auto i = 0; i < MaxNumPresetVmas; ++i) {
            auto const &region = NAMED_PRESET_VMAS[i];
            auto vma = ustl::make_rc<VmArea>(
                construct_at(region.altvma, kaspace, region.base, region.size, region.rights, region.flags, region.name)
            );
            vma->activate();
        }
    }

    INIT_CODE
    static auto init_heap() -> void
    {

    }

    /// Used in init_vmm_postheap
    INIT_DATA
    static PresetVmaInfo KERNEL_VMAS[] = {
        {
            .name = "k:code",
            .base = VirtAddr(KERNEL_CODE_START),
            .size = usize(KERNEL_CODE_END - KERNEL_CODE_START),
            .rights = MmuFlags::Executable | MmuFlags::Readable | MmuFlags::Cached
        },
        {
            .name = "k:data",
            .base = VirtAddr(KERNEL_DATA_START),
            .size = usize(KERNEL_DATA_END - KERNEL_DATA_START),
            .rights = MmuFlags::Writable | MmuFlags::Readable | MmuFlags::Cached
        },
        {
            .name = "k:rodata",
            .base = VirtAddr(KERNEL_RODATA_START),
            .size = usize(KERNEL_RODATA_END - KERNEL_RODATA_START),
            .rights = MmuFlags::Readable | MmuFlags::Cached
        },
        {
            .name = "k:bss",
            .base = VirtAddr(KERNEL_BSS_START),
            .size = usize(KERNEL_BSS_END - KERNEL_BSS_START),
            .rights = MmuFlags::Readable | MmuFlags::Cached
        },
        {
            .name = "k:init",
            .base = VirtAddr(KERNEL_INIT_START),
            .size = usize(KERNEL_INIT_END - KERNEL_INIT_START),
            .rights = MmuFlags::Readable | MmuFlags::Writable | MmuFlags::Executable
        },
    };

    INIT_CODE
    static auto init_vmm_postheap() -> void
    {
        auto kaspace = VmAspace::kernel_aspace();

        if constexpr (OURS_CONFIG_KASLR) {
            VirtAddr const link_load_addr = VirtAddr(KERNEL_IMAGE_START);
            VirtAddr const real_load_addr = get_kernel_virt_base();
            isize const delta = real_load_addr - link_load_addr;

            for (auto i = 0; i < MaxNumPresetVmas; ++i) {
                KERNEL_VMAS[i].base += delta;
            }
        }

        for (auto &region: KERNEL_VMAS) {
            kaspace->root_area().reserve_subvma(region.name, region.base, region.size, region.rights);
        }
    }

    /// Requires:
    ///     1). 
    INIT_CODE 
    auto init_vmm() -> void 
    {  
        init_vmm_preheap();

        init_heap();
        // Here `Scope<T>` is available

        init_vmm_postheap();
    }
}
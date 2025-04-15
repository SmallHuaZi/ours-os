#include <ours/mem/init.hpp>

#include <ours/mem/vmm.hpp>
#include <ours/mem/vm_aspace.hpp>
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

using ustl::algorithms::min;
using ustl::algorithms::max;
using ustl::algorithms::clamp;
using ustl::mem::construct_at;

namespace ours::mem {
    struct PresetVmaInfo {
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
    static ustl::LazyInit<VmArea> s_named_preset_vmas[MaxNumPresetVmas];

    CXX11_CONSTEXPR
    static VmaFlags const PRESET_VMAF = VmaFlags::Mapped;

    CXX11_CONSTEXPR
    static MmuFlags const PRESET_MMUF = MmuFlags::PermMask;

    INIT_DATA
    static PresetVmaInfo s_preset_vmas[] = {
        {
            .name = "k:image",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = VirtAddr(kImageStart),
            .size = usize(kImageEnd - kImageStart),
            .rights = PRESET_MMUF,
            .flags = VmaFlags::Mapped,
            .altvma = s_named_preset_vmas[KernelImageVma].data(),
        },
        {
            .name = "k:physmap",
            // If ASLR(Address space layout randomilization) enabled, this needs to fix up with get_kernel_virt_base().
            .base = PhysMap::kVirtBase,
            .size = PhysMap::kSize,
            .rights = PRESET_MMUF,
            .flags = VmaFlags::Mapped,
            .altvma = s_named_preset_vmas[PhysMapVma].data(),
        },
        {
            .name = "k:padding",
            .base = PhysMap::kVirtBase + PhysMap::kSize,
            // Reserve a max page to prevent over prevent overwring.
            .size = MAX_PAGE_SIZE,
            .rights = PRESET_MMUF,
            .altvma = s_named_preset_vmas[PaddingVma].data(),
        },
    };

    INIT_CODE
    static auto init_vmm_preheap() -> void {
        VmAspace::init_kernel_aspace();
        auto kaspace = VmAspace::kernel_aspace();

        if constexpr (OURS_CONFIG_KASLR) {
            VirtAddr const link_load_addr = VirtAddr(kImageStart);
            VirtAddr const real_load_addr = get_kernel_virt_base();
            isize const delta = real_load_addr - link_load_addr;

            for (auto i = 0; i < MaxNumPresetVmas; ++i) {
                s_preset_vmas[i].base += delta;
            }
        }

        // Initialize all of named vmas.
        for (auto i = 0; i < MaxNumPresetVmas; ++i) {
            auto const &region = s_preset_vmas[i];
            auto vma = ustl::make_rc<VmArea>(
                construct_at(region.altvma, kaspace, region.base, region.size, region.rights, region.flags, region.name)
            );
            vma->activate();
        }
    }

    INIT_CODE
    static auto init_heap() -> void {

    }

    /// Used in init_vmm_postheap
    INIT_DATA
    static PresetVmaInfo s_kernel_vmas[] = {
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
    };

    INIT_CODE
    static auto init_vmm_postheap() -> void {
        auto kaspace = VmAspace::kernel_aspace();

        if constexpr (OURS_CONFIG_KASLR) {
            VirtAddr const link_load_addr = VirtAddr(kImageStart);
            VirtAddr const real_load_addr = get_kernel_virt_base();
            isize const delta = real_load_addr - link_load_addr;

            for (auto i = 0; i < MaxNumPresetVmas; ++i) {
                s_kernel_vmas[i].base += delta;
            }
        }

        for (auto &region: s_kernel_vmas) {
            kaspace->root_area().reserve_subvma(region.name, region.base, region.size, region.rights);
        }
    }

    /// Requires:
    ///     1). 
    INIT_CODE 
    auto init_vmm() -> void {
        init_vmm_preheap();

        init_heap();
        // Here `Scope<T>` is available

        init_vmm_postheap();
    }
}
#include <ours/phys/kernel-boot.hpp>
#include <ours/phys/kernel-package.hpp>
#include <ours/phys/handoff.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <arch/random.hpp>
#include <arch/tick.hpp>
#include <omitl/obi-header.hpp>

using ustl::mem::align_up;

namespace ours::phys {
    auto KernelBoot::boot(KernelPackage &kpackage) -> void {
        auto const package = kpackage.package();
        auto const pkgsize = package.size();
        auto const pkgaddr = package.data();

        auto const kheader = reinterpret_cast<omitl::KernelHeader *>(pkgaddr);
        DEBUG_ASSERT(kheader->signature == OMI_KERNEL_SIGNATURE, "Invalid kernel signature");

        dprintln("kernel entry point: 0x{:X}", kheader->entry_point);
        dprintln("kernel reserved size: 0x{:X}", kheader->reserved_size);

        kernel_entry_= kheader->entry_point;
        kernel_size_ = pkgsize - sizeof(*kheader) + kheader->reserved_size;
        kernel_addr_ = reinterpret_cast<PhysAddr>(pkgaddr);

        // Do not move this to `KernelBoot::unpack_kernel()`.
        // In the near future, the package will be replaced with a new implementation.
        DEBUG_ASSERT(ustl::mem::is_aligned(kernel_addr_, PAGE_SIZE), "Kernel package must be aligned");
        ustl::algorithms::copy_n(
            reinterpret_cast<u8 *>(kernel_addr_ + sizeof(omitl::KernelHeader)),
            pkgsize - sizeof(omitl::KernelHeader),
            reinterpret_cast<u8 *>(kernel_addr_)
        );

        build_kernel_aspace();
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto calc_kernel_load_addr(usize kernel_size) -> VirtAddr {
        auto const kaslr_range_start = KERNEL_ASPACE_BASE + ARCH_PHYSMAP_SIZE + GB(1);
        auto const kaslr_range_end = kaslr_range_start + GB(1);

        usize offset;
        if (!arch::rand(&offset)) {
            offset = arch::Tick::get().tsc;
        }
        auto const unaligned_load_addr = kaslr_range_start + (offset % (kaslr_range_end - kaslr_range_start));
        return align_up(unaligned_load_addr, MAX_PAGE_SIZE);
    }

    auto KernelBoot::build_kernel_aspace() -> void {
        auto aspace = global_aspace();
        DEBUG_ASSERT(aspace, "Global aspace must be initialized");

        CXX11_CONSTEXPR
        auto const mmuf = mem::MmuFlags::PermMask | mem::MmuFlags::Present;
        auto result = aspace->map(KERNEL_ASPACE_BASE, ARCH_PHYSMAP_SIZE / PAGE_SIZE, 0, mmuf);
        DEBUG_ASSERT(result.is_ok(), "Failed to map physmap area");

        auto const kernel_load_addr = calc_kernel_load_addr(kernel_size_);
        auto const nr_pages = align_up(kernel_size_, MAX_PAGE_SIZE) / PAGE_SIZE;
        result = aspace->map(kernel_load_addr, nr_pages, kernel_addr_, mmuf);
        DEBUG_ASSERT(result.is_ok(), "Failed to map main module");

        kernel_entry_ += kernel_load_addr;
    }

    WEAK
    auto KernelBoot::arch_do_handoff() -> void {
    }

    struct KernelEntry {
        template <typename... Args>
        FORCE_INLINE CXX11_CONSTEXPR
        static auto get(usize entry_point, Args&&... args) -> void {
            using Type = auto (*)(Args...) -> void;
            return reinterpret_cast<Type>(entry_point);
        }
    };

    auto KernelBoot::do_handoff() -> void {
        auto const handoff = global_handoff();
        DEBUG_ASSERT(handoff, "Global handoff must be initialized");

        auto const mem = global_bootmem();
        handoff->mem.bootmem = {
            .protect = std::bind_front(&BootMem::protect, mem),
            .allocate = std::bind_front(&BootMem::allocate_bounded, mem),
            .deallocate = std::bind_front(&BootMem::deallocate, mem),
            .iterate = std::bind_front(&BootMem::iterate, mem),
            .start_address = mem->start_address(),
            .end_address = mem->end_address(),
        };

        handoff->mem.kernel_load_addr = kernel_addr_;

        start_kernel(handoff);
    }

} // namespace ours::phys
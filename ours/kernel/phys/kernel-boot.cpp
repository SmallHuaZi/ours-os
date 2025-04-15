#include <ours/phys/kernel-boot.hpp>
#include <ours/phys/kernel-image.hpp>
#include <ours/phys/kernel-package.hpp>
#include <ours/phys/handoff.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ours/phys/boot-options.hpp>

#include <ours/arch/aspace_layout.hpp>

#include <arch/random.hpp>
#include <arch/tick.hpp>
#include <omitl/obi-header.hpp>
#include <eftl/elf/views.hpp>

namespace ours::phys {
    auto KernelBoot::gen_load_bias(usize const limit) -> usize {
        usize offset;
        if (!arch::rand(&offset)) {
            offset = arch::Tick::get().tsc;
        }
        return offset % limit;
    }

    auto KernelBoot::init(KernelPackage &kpackage) -> void {
        CXX11_CONSTEXPR
        static auto const kMetaDataSize = sizeof(omitl::ObiExeSign);
        auto const package = kpackage.package();
        auto const exe_signature = *reinterpret_cast<omitl::ObiExeSign *>(package.data());

        // FIXME(SmallHuaZi) a way transition to the impletation of InitFs
        initfs_ = InitFs(package.data() + kMetaDataSize, package.size() - kMetaDataSize);
        // TODO(SmallHuaZi) Base on the passed option selecting target to load.
        // auto mmod = initfs_.read(global_boot_options()->load_target);
        auto mmod = initfs_;

        if (exe_signature == omitl::ObiExeSign::Raw) {
            auto status = kimage_.init(KernelImage::Type::RawImage, mmod);
            if (status != Status::Ok) {
                panic("Failed to initialize an raw image: {}", to_string(status));
            }
        } else if (exe_signature == omitl::ObiExeSign::Elf) {
            auto status = kimage_.init(KernelImage::Type::ElfImage, mmod);
            if (status != Status::Ok) {
                panic("Failed to initialize an elf image: {}", to_string(status));
            }
        } else {
            panic("Unrecognized executable type");
        }

        if (!global_boot_options()->aslr_disabled) {
            kaslr_offset_ = gen_load_bias(GB(1));
            // Do check
        } else {
            kaslr_offset_ = 0;
        }
    }

    auto KernelBoot::load() -> void {
        auto const image_size = ustl::mem::align_up(kimage_.size(), PAGE_SIZE);
        auto const mem = global_bootmem();
        PhysAddr target_addr = mem->allocate(image_size, kMaxPageAlign);
        if (!target_addr) {
            panic("No enough memory to place image");
        }
        ustl::algorithms::fill_n(reinterpret_cast<usize *>(target_addr), image_size / sizeof(usize), 0);

        auto result = kimage_.load_at(target_addr);
        if (result.is_err()) {
            panic("Failed to load image at {}, given a reason: {}", target_addr, to_string(result.unwrap_err()));
        }

        kernel_entry_ = result.unwrap();
        kernel_size_ = image_size;
        kernel_addr_ = target_addr;
    }

    auto KernelBoot::build_kernel_aspace() -> void {
        CXX11_CONSTEXPR
        auto const kDefaultKernelMmuFlags = mem::MmuFlags::PermMask | mem::MmuFlags::Present;

        auto aspace = global_aspace();
        DEBUG_ASSERT(aspace, "Global aspace must be initialized");

        auto const nr_pages = ustl::mem::align_up(kernel_size_, PAGE_SIZE);
        auto result = aspace->map(KERNEL_LOAD_BASE + kaslr_offset_, nr_pages , kernel_addr_, kDefaultKernelMmuFlags);
        DEBUG_ASSERT(result.is_ok(), "Failed to map kernel image");

        kernel_entry_  = kernel_entry_ - kernel_addr_ + KERNEL_LOAD_BASE + kaslr_offset_;

        result = aspace->map(KERNEL_ASPACE_BASE, ARCH_PHYSMAP_SIZE / PAGE_SIZE, 0, kDefaultKernelMmuFlags);
        DEBUG_ASSERT(result.is_ok(), "Failed to map physmap area");

        auto fix_status = kimage_.fix_mapping(*aspace);
        DEBUG_ASSERT(fix_status == Status::Ok, "Failed to fix kernel mapping: {}", to_string(fix_status));
    }

    auto KernelBoot::boot() -> void {
        build_kernel_aspace();
        do_handoff();
    }

    WEAK_LINK
    auto KernelBoot::arch_do_handoff() -> void 
    {}

    auto KernelBoot::do_handoff() -> void {
        auto const handoff = global_handoff();
        DEBUG_ASSERT(handoff, "Global handoff must be initialized");

        auto const mem = global_bootmem();
        handoff->mem.bootmem = {
            .protect = std::bind_front(&BootMem::protect, mem),
            .set_node = std::bind_front(&BootMem::set_node, mem),
            .allocate = std::bind_front(&BootMem::allocate_bounded, mem),
            .deallocate = std::bind_front(&BootMem::deallocate, mem),
            .iterate = std::bind_front(&BootMem::iterate, mem),
            .min_address = mem->start_address(),
            .max_address = mem->end_address(),
        };

        handoff->mem.kernel_load_addr = kernel_addr_;

        invoke_kernel(handoff);
    }

} // namespace ours::phys
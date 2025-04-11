/// `PhysLoad` is mainly to load the main module and other bootable images.
#include <ours/phys/init.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/handoff.hpp>
#include <ours/phys/kernel-package.hpp>
#include <ours/phys/kernel-boot.hpp>
#include <ours/phys/boot-options.hpp>
#include <ours/phys/print.hpp>

namespace ours::phys {
    Handoff g_handoff;
    BootOptions g_boot_options;

    FORCE_INLINE
    static auto patch_kernel(KernelImage &image) -> void {
    }

    FORCE_INLINE
    static auto relocate_kernel(KernelImage &image, usize load_address) -> void {
        image.relocate(load_address);

        dprintln("Relocate kenrel with offset=0x{:X}", load_address);
    }

    FORCE_INLINE
    static auto load_kernel(KernelBoot &kboot) -> void {
        kboot.load();

        dprintln("Kernel image entry point: 0x{:X}", kboot.kernel_entry_point());
        dprintln("Kernel image start address: 0x{:X}", kboot.kernel_addr());
        dprintln("Kernel image size: 0x{:X}", kboot.kernel_size());
    }

    NO_RETURN
    auto obi_main(PhysAddr obi) -> void {
        // For debug
        g_boot_options.aslr_disabled = true;

        Aspace aspace;
        init_memory(obi, &aspace);

        KernelPackage kpackage;
        kpackage.init(obi);

        KernelBoot kboot;
        kboot.init(kpackage);

        load_kernel(kboot);

        auto &kimage = kboot.image();
        patch_kernel(kimage);

        relocate_kernel(kimage, kboot.load_address());

        dprintln("Handoff is at 0x{:X}", usize(global_handoff()));
        kboot.boot();

        panic("Never reach at here");
    }

} // namespace ours::phys
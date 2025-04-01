/// `PhysLoad` is mainly to load the main module and other bootable images.
#include <ours/phys/init.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/handoff.hpp>
#include <ours/phys/kernel-package.hpp>
#include <ours/phys/kernel-boot.hpp>

namespace ours::phys {
    Handoff g_handoff;

    NO_RETURN
    auto obi_main(PhysAddr obi) -> void {
        Aspace aspace;
        init_memory(obi, &aspace);

        KernelPackage kpackage;
        kpackage.init(obi);

        KernelBoot kboot;
        kboot.boot(kpackage);

        panic("Never reach at here");
    }

} // namespace ours::phys
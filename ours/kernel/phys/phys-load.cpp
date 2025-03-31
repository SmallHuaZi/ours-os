/// `PhysLoad` is mainly to load the main module and other bootable images.
#include <ours/phys/init.hpp>
#include <ours/phys/aspace.hpp>
#include <ours/phys/handoff.hpp>
#include <ours/phys/kernel-package.hpp>

namespace ours::phys {
    Handoff  *gHandoff;

    NO_RETURN
    auto obi_main(PhysAddr obi) -> void {
        Aspace aspace;
        init_memory(obi, &aspace);

        KernelPackage kpackage;
        kpackage.init(obi);

        panic("Never reach at here");
    }

} // namespace ours::phys
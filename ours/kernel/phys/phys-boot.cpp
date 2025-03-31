/// `PhysBoot` is mainly to boot the main module and other bootable images.
/// It's most work are prepare the data of kernel initialization.

#include <ours/config.hpp>
#include <ours/phys/kernel-package.hpp>

namespace ours::phys {
    NO_RETURN
    auto boot_main_module(KernelPackage &kpackage) -> void {
    }

} // namespace ours::phys
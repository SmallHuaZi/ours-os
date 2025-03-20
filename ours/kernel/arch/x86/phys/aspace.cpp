#include <ours/phys/aspace.hpp>
#include <arch/system.hpp>
#include <arch/tlb.hpp>

namespace ours::phys {
    auto Aspace::arch_install() const -> void {
        auto cr4 = arch::Cr4::read();
        cr4.set<cr4.Pge>(0);
        arch::Cr3::from_value(pgd_).write();
        cr4.set<cr4.Pge>(1);
    }

} // namespace ours::phys
#include <ours/arch/x86/page_table.hpp>
#include <ours/arch/cpu.hpp>

#include <arch/tlb.hpp>

namespace ours::mem {
    auto X86MmuPageSynchroniser::sync(arch::paging::PendingInvalidationItems const &items) -> void {
        auto const pcpu_sync_task = [&] (Self *context) {
            auto const n = items.count();
            for (auto i = 0; i < n; ++i) {
                arch::x86_invlpg(items.items_[i].get<items.Addr>());
            }
        };
    }

} // namespace ours::mem
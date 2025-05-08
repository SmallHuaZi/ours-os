#include <ours/arch/thread.hpp>
#include <ours/arch/x86/entry.hpp>
#include <ours/arch/x86/descriptor.hpp>
#include <ours/arch/x86/feature.hpp>

#include <ours/task/thread.hpp>

#include <arch/x86/fsgs.hpp>
#include <arch/x86/msr.hpp>

#include <ustl/mem/container_of.hpp>

using namespace arch;

namespace ours::task {
    NO_MANGLE CPU_LOCAL
    X86Thread *X86Thread::s_current_arch_thread;

    static auto swap_gsbase_and_fsbase(usize new_gsbase, usize new_fsbase, 
                                       usize *old_gs_base, usize *old_fs_base) -> void {
        if (g_feature_has_fsgsbase) {
            *old_fs_base = read_fsbase<usize>();
        } else {
            *old_fs_base = MsrIo::read<usize>(MsrRegAddr::IA32FsBase);
        }

        if (g_feature_has_fsgsbase) {
            write_fsbase(new_fsbase);
            *old_gs_base = exchange_gsbase_after_swap(new_gsbase);
        } else {
            *old_gs_base = MsrIo::read<usize>(MsrRegAddr::IA32GsBase);
            MsrIo::write(MsrRegAddr::IA32GsBase, new_gsbase);
            MsrIo::write(MsrRegAddr::IA32FsBase, new_fsbase);
        }
    }

    auto X86Thread::switch_context(Self *prev, Self *next) -> void {
        // Set stack pointer which was used at privilege 0.
        x86_set_tss_sp(Thread::from_arch_thread(next)->kernel_stack().top());

        swap_gsbase_and_fsbase(next->gs_base_, next->fs_base_, &prev->gs_base_, &prev->fs_base_);

        set_current_thread(next);

        x86_switch_context(&prev->sp_, next->sp_);
    }

} // namespace ours::task
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
        g_feature_has_fsgsbase = false;
        if (g_feature_has_fsgsbase) {
            *old_fs_base = read_fsbase<usize>();
        } else {
            *old_fs_base = MsrIo::read<usize>(MsrRegAddr::IA32FsBase);
        }

        if (g_feature_has_fsgsbase) {
            write_fsbase(new_fsbase);
            *old_gs_base = exchange_gsbase_after_swap(new_gsbase);
        } else {
            MsrIo::write(MsrRegAddr::IA32FsBase, new_fsbase);

            *old_gs_base = MsrIo::read<usize>(MsrRegAddr::IA32KernelGsBase);
            MsrIo::write(MsrRegAddr::IA32KernelGsBase, new_gsbase);
        }
    }

    auto X86Thread::switch_context(Self *prev, Self *next) -> void {
        // Set stack pointer which was used at privilege 0.
        x86_set_tss_sp(Thread::of(next)->kernel_stack().top());

        swap_gsbase_and_fsbase(next->gs_base_, next->fs_base_, &prev->gs_base_, &prev->fs_base_);

        set_current_thread(next);

        x86_switch_context(&prev->sp_, next->sp_);
    }

    auto X86Thread::init(VirtAddr entry_point) -> void {
        struct x64ContextSwitchFrameLayout {
            u64 r15;
            u64 r14;
            u64 r13;
            u64 r12;
            u64 rbp;
            u64 rbx;
            u64 rip;
        };

        auto thread = task::Thread::of(this);
        auto stack_top = thread->kernel_stack().top();

        DEBUG_ASSERT(ustl::mem::is_aligned(stack_top, 16));

        // Before to construct a frame on stack, we must preserve 8 byte size of space
        // for caller's the return address.
        auto frame = reinterpret_cast<x64ContextSwitchFrameLayout *>(stack_top - 8);
        frame--;
        memset(frame, 0, sizeof(*frame));

        frame->rip = entry_point;
        sp_ = reinterpret_cast<VirtAddr>(frame);

        fs_base_ = 0;
        gs_base_ = 0;
    }

} // namespace ours::task
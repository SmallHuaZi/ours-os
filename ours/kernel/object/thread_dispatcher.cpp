#include <ours/object/thread_dispatcher.hpp>
#include <ours/object/process_dispatcher.hpp>

#include <ours/arch/entry.hpp>
#include <ours/task/thread.hpp>

#include <ktl/new.hpp>

namespace ours::object {
    auto ThreadDispatcher::spawn(ustl::Rc<ProcessDispatcher> process, char const *name, 
                                 KernelHandle<Self> *out) -> Status {
        auto self = ustl::make_rc<Self>(new (mem::kGafUser) Self(process, name));
        if (!self) {
            return Status::OutOfMem;
        }

        auto core_thread = task::Thread::spawn(name, 0, Self::trampoline, self.as_ptr_mut());
        if (!core_thread) {
            return Status::OutOfMem;
        }

        self->kernel_thread_ = core_thread;
        return Status::Ok;
    }

    auto ThreadDispatcher::trampoline(Self *self) -> i32 {
        DEBUG_ASSERT(self->user_entry_);
        auto &task = *self->user_entry_;

        arch::IrqFrame frame{};
        arch_build_uspace_irqframe(&frame, task.ip, task.sp, task.arg1, task.arg2);

        arch_enter_uspace(&frame);
        unreachable();
    }

    auto ThreadDispatcher::start(TaskState const &state) -> Status {
        canary_.verify();

        user_entry_ = state;
        auto status = process_->attach_thread(*this);
        if (Status::Ok != status) {
            user_entry_ = ustl::none();
            return status;
        }

        return Status::Ok;
    }

    auto ThreadDispatcher::activate(bool suspend) -> Status {
        kernel_thread_->bind_user_thread(ustl::make_rc<Self>(this));

        if (suspend) {
            kernel_thread_->suspend();
        } else {
            kernel_thread_->resume();
        }
        return Status::Ok;
    }

} // namespace ours::object
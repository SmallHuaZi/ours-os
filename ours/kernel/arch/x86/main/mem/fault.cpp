#include <ours/arch/x86/faults.hpp>
#include <ours/mem/fault.hpp>

#include <arch/x86/interrupt.hpp>
#include <arch/macro/mmu.hpp>
#include <arch/system.hpp>

#include <logz4/log.hpp>

namespace ours {
    using arch::Cr2;
    using mem::VmfCause;

    auto x86_handle_page_fault(arch::IrqFrame *frame) -> Status {
        usize const error_code = frame->error_code;
        if (!(PFEX_MASK & error_code)) {
            return Status::Unsupported;
        }

        VirtAddr const addr = Cr2::read().get<Cr2::Address>();

        VmfCause cause{};
        cause |= (error_code & PFEX_PRESENT) ? VmfCause::Absent : VmfCause::None;
        cause |= (error_code & PFEX_WRITE) ? VmfCause::Write : VmfCause::None;
        cause |= (error_code & PFEX_USER) ? VmfCause::User : VmfCause::None;

        // Call the top level page fault handling routine.
        auto const status = mem::handle_vm_fault(addr, cause);
        if (Status::Ok != status) {
            return status;
        }

        return Status::Ok;
    }

} // namespace ours
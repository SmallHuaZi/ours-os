#include <ours/arch/x86/faults.hpp>
#include <arch/x86/interrupt.hpp>

namespace ours {
    static auto x86_dispatch_exception(arch::IrqVector vector, arch::IrpFrame *frame) -> void
    {
        if (vector == arch::IrqVector::PageFault) {
            if (Status::Ok != x86_handle_page_fault(frame)) {
            }
        }
            
        switch (vector) {
            case arch::IrqVector::DivideError:
            case arch::IrqVector::Debug:
            case arch::IrqVector::Nmi:
            case arch::IrqVector::Breakpoint:
            case arch::IrqVector::Overflow:
            case arch::IrqVector::BoundRangeExceeded:
            case arch::IrqVector::InvalidOpcode:
            case arch::IrqVector::DeviceNotAvailable:
            case arch::IrqVector::DoubleFault:
            case arch::IrqVector::CoprocessorSegmentOverrun:
            case arch::IrqVector::InvalidTss:
            case arch::IrqVector::SegmentNotPresent:
            case arch::IrqVector::StackFaultException:
            case arch::IrqVector::GeneralProtection:
            case arch::IrqVector::X87FloatingPoint:
            case arch::IrqVector::AlignmentCheck:
            case arch::IrqVector::MachineCheck:
            case arch::IrqVector::SimdFloatingPoint:
            case arch::IrqVector::Virtualizatoin:
            case arch::IrqVector::ControlProtection:
                break;
        }
    }

    NO_MANGLE
    auto arch_handle_exception(arch::IrqVector vector, arch::IrpFrame *frame) -> void
    {}

} // namespace ours
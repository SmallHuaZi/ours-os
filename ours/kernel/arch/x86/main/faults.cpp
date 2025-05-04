#include <ours/arch/x86/faults.hpp>
#include <arch/x86/interrupt.hpp>
#include <ours/platform/init.hpp>

#include <logz4/log.hpp>

namespace ours {
    static auto x86_dispatch_exception(arch::IrqVec vector, arch::IrqFrame *frame) -> void {
        if (vector == arch::IrqVec::PageFault) {
            if (Status::Ok != x86_handle_page_fault(frame)) {
            }
        }
            
        switch (vector) {
            case arch::IrqVec::DivideError:
            case arch::IrqVec::Debug:
            case arch::IrqVec::Nmi:
            case arch::IrqVec::Breakpoint:
            case arch::IrqVec::Overflow:
            case arch::IrqVec::BoundRangeExceeded:
            case arch::IrqVec::InvalidOpcode:
            case arch::IrqVec::DeviceNotAvailable:
            case arch::IrqVec::DoubleFault:
            case arch::IrqVec::CoprocessorSegmentOverrun:
            case arch::IrqVec::InvalidTss:
            case arch::IrqVec::SegmentNotPresent:
            case arch::IrqVec::StackFaultException:
            case arch::IrqVec::GeneralProtection:
            case arch::IrqVec::X87FloatingPoint:
            case arch::IrqVec::AlignmentCheck:
            case arch::IrqVec::MachineCheck:
            case arch::IrqVec::SimdFloatingPoint:
            case arch::IrqVec::Virtualizatoin:
            case arch::IrqVec::ControlProtection:
                break;
            
            case arch::IrqVec::PlatformIrqMin ... arch::IrqVec::PlatformIrqMax:
                break;

            case arch::IrqVec::ApicTimer:
                log::info("APIC-PM Tick");
                break;
        }
    }

    NO_MANGLE
    auto arch_handle_exception(arch::IrqFrame *frame) -> void {
        x86_dispatch_exception(frame->vector, frame);
    }

} // namespace ours
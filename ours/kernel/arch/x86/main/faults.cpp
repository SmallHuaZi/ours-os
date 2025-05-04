#include <ours/arch/x86/faults.hpp>
#include <arch/x86/interrupt.hpp>
#include <ours/platform/init.hpp>
#include <ours/platform/interrupt.hpp>

#include <logz4/log.hpp>

namespace ours {
    FORCE_INLINE
    static auto irqvec_to_irqnum(arch::IrqVec vector) -> HIrqNum {
        DEBUG_ASSERT(static_cast<HIrqNum>(vector) >= static_cast<HIrqNum>(arch::IrqVec::PlatformIrqMin));
        return static_cast<HIrqNum>(vector) - static_cast<HIrqNum>(arch::IrqVec::PlatformIrqMin);
    }

    static auto x86_dispatch_exception(arch::IrqVec vector, arch::IrqFrame *frame) -> void {
        using namespace arch;
        if (vector == IrqVec::PageFault) {
            if (Status::Ok != x86_handle_page_fault(frame)) {
            }
        }
            
        switch (vector) {
            case IrqVec::DivideError:
            case IrqVec::Debug:
            case IrqVec::Nmi:
            case IrqVec::Breakpoint:
            case IrqVec::Overflow:
            case IrqVec::BoundRangeExceeded:
            case IrqVec::InvalidOpcode:
            case IrqVec::DeviceNotAvailable:
            case IrqVec::DoubleFault:
            case IrqVec::CoprocessorSegmentOverrun:
            case IrqVec::InvalidTss:
            case IrqVec::SegmentNotPresent:
            case IrqVec::StackFaultException:
            case IrqVec::GeneralProtection:
            case IrqVec::X87FloatingPoint:
            case IrqVec::AlignmentCheck:
            case IrqVec::MachineCheck:
            case IrqVec::SimdFloatingPoint:
            case IrqVec::Virtualizatoin:
            case IrqVec::ControlProtection:
                break;
            
            case IrqVec::PlatformIrqMin ... IrqVec::PlatformIrqMax:
                platform_handle_irq(irqvec_to_irqnum(vector), frame);
                break;

            case IrqVec::ApicTimer:
                log::info("APIC-PM Tick");
                break;
        }
    }

    NO_MANGLE
    auto arch_handle_exception(arch::IrqFrame *frame) -> void {
        x86_dispatch_exception(frame->vector, frame);
    }

} // namespace ours
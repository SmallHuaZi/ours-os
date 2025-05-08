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
            ASSERT(Status::Ok != x86_handle_page_fault(frame));
        }
            
        switch (vector) {
            case IrqVec::DivideError:
                log::info("#DE");
                break;
            case IrqVec::Debug:
                log::info("#DB");
                break;
            case IrqVec::Nmi:
                log::info("#NMI");
                break;
            case IrqVec::Breakpoint:
                log::info("#BK");
                break;
            case IrqVec::Overflow:
                log::info("#OF");
                break;
            case IrqVec::BoundRangeExceeded:
                log::info("#BE");
                break;
            case IrqVec::InvalidOpcode:
                log::info("#IO");
                break;
            case IrqVec::DeviceNotAvailable:
                log::info("#DNA");
                break;
            case IrqVec::DoubleFault:
                log::info("#DF");
                break;
            case IrqVec::CoprocessorSegmentOverrun:
                log::info("#CSO");
                break;
            case IrqVec::InvalidTss:
                log::info("#IT");
                break;
            case IrqVec::SegmentNotPresent:
                log::info("#SNP");
                break;
            case IrqVec::StackFaultException:
                log::info("#SF");
                break;
            case IrqVec::GeneralProtection:
                log::info("#GP");
                break;
            case IrqVec::X87FloatingPoint:
                log::info("#X86FP");
                break;
            case IrqVec::AlignmentCheck:
                log::info("#AC");
                break;
            case IrqVec::MachineCheck:
                log::info("#MC");
                break;
            case IrqVec::SimdFloatingPoint:
                log::info("#SIMDFP");
                break;
            case IrqVec::Virtualizatoin:
                log::info("#V");
                break;
            case IrqVec::ControlProtection:
                log::info("#CP");
                break;
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
#include <ours/mem/fault.hpp>
#include <ours/mem/vm_aspace.hpp>

#include <ours/task/thread.hpp>

namespace ours::mem {
    NO_MANGLE
    auto handle_vm_fault(VirtAddr addr, VmfCause cause) -> Status 
    {
        // 1. Checks flags and rights.
        // 2. Judge the address belong to user address space or kernel.  
        // 3. Locate the VmRootArea that address belong to.
        // 4. If no VmRootArea found, checks if the stack overflows.
        VmAspace *aspace = task::Thread::Current::aspace();
        aspace->fault(addr, cause);

        return Status::Ok;
    }
}
#include <ours/arch/cpu.hpp>

namespace ours {
    auto arch_current_cpu() -> CpuId
    {  return 0; }

    auto arch_activate_cpu(CpuId) -> Status
    {  return Status::Unimplemented; }

    auto arch_deactivate_cpu(CpuId) -> Status
    {  return Status::Unimplemented; }

} // namespace ours
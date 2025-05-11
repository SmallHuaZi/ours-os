#include <ours/cpu-states.hpp>
#include <ours/cpu-local.hpp>

namespace ours {
    auto set_current_cpu_online(bool online) -> void {
        global_cpu_states().set_online(CpuLocal::cpunum(), online);
    }

} // namespace ours
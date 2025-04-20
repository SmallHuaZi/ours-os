#include <ours/cpu-local.hpp>
#include <ours/tests/test.hpp>
#include <ours/assert.hpp>

#include <gktl/init_hook.hpp>

namespace ours::test {
    OTEST_ABI
    static auto check_magic() -> void {
        CXX11_CONSTEXPR
        static const int Magic = 0x3D4C5E7A;

        CPU_LOCAL 
        static int s_magic = Magic;

        CpuLocal::for_each(&s_magic, [] (int &magic, CpuNum cpunum) {
            DEBUG_ASSERT(magic == Magic, "Invalid magic");
            DEBUG_ASSERT(CpuLocal::check_addr(VirtAddr(&magic), cpunum));
        });
    }
    GKTL_INIT_HOOK(CpuLocalMagicTest, check_magic, gktl::InitLevel::CpuLocal);

} // namespace ours::test
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/memory_model.hpp>

#include <logz4/log.hpp>
#include <gktl/counter.hpp>
#include <gktl/command.hpp>

namespace ours::mem {
    GKTL_COUNTER(EACH_ROLE_FRAMES_COUNTER, )

    auto PmFrameBase::dump() const -> void {
        log::info("PmFrame at 0x{:0>8X}: [role: {:5}, zone: {:6}, section: {}, node: {}, pa: 0x{:0>8X}]",
            usize(this),
            to_string(role()),
            to_string(zone()),
            secnum(),
            nid(),
            frame_to_phys(static_cast<PmFrame const *>(this))
        );
    }

    static auto cmd_dump_frame(int argc, char **argv) -> int {
    }

    GKTL_COMMAND(FrameDumper, "dump-frame", cmd_dump_frame, "");
}
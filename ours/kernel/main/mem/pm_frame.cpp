#include <ours/mem/pm_frame.hpp>
#include <ours/mem/memory_model.hpp>

#include <logz4/log.hpp>
#include <gktl/counter.hpp>
#include <gktl/command.hpp>

namespace ours::mem {
    auto PageFrameBase::dump() const -> void {
        log::info("PmFrame at 0x{:0>8X}:", usize(this));
        log::info("  [role: {:5} |zone: {:6} |section: {} |node: {} |pa: 0x{:0>8X}]",
            to_string(role()),
            to_string(zone()),
            secnum(),
            nid(),
            frame_to_phys(this)
        );
    }

    static auto cmd_dump_frame(int argc, char **argv) -> int {
    }

    GKTL_COMMAND(FrameDumper, "dump-frame", cmd_dump_frame, "");
}
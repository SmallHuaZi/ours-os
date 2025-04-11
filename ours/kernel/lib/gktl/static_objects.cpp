#include <gktl/static_objects.hpp>

namespace gktl {
    typedef void (*Ctor)();
    extern Ctor const CTORS_START[] LINK_NAME("__ctors_start");
    extern Ctor const CTORS_END[] LINK_NAME("__ctors_end");

    INIT_CODE
    auto init_static_objects() -> void {
        auto const n = CTORS_END - CTORS_START;
        for (auto ctor = CTORS_START; ctor != CTORS_END; ++ctor) {
            (*ctor)();
        }
    }
} // namespace gktl
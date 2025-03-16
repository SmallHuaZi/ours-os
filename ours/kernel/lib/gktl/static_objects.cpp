#include <gktl/static_objects.hpp>

namespace gktl {
    typedef void (*Ctor)();
    extern Ctor const CTORS_START[] LINK_NAME("__ctors_start");
    extern Ctor const CTORS_END[] LINK_NAME("__ctors_end");

    INIT_CODE
    auto init_static_objects() -> void {
        for (auto ctor = CTORS_START; ctor != CTORS_END; ++ctor) {
            (*ctor)();
        }
    }
} // namespace gktl
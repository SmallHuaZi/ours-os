#include <gktl/static_objects.hpp>

namespace gktl {
    typedef void (*Ctor)();
    NO_MANGLE Ctor const CTORS_START[];
    NO_MANGLE Ctor const CTORS_END[];

    INIT_CODE
    auto init_static_objects() -> void
    {
        for(auto i = CTORS_START; i != CTORS_END; ++i) {
            (*i)();
        }
    }
}
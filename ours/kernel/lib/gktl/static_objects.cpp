#include <gktl/static_objects.hpp>

namespace gktl {
    EARLY_CODE
    auto init_static_objects() -> void
    {
        for(auto i = CTORS_START; i != CTORS_END; ++i) {
            (*i)();
        }
    }
}
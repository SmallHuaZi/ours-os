#include <omi/obi.hpp>
#include <ours/assert.hpp>

namespace omi {
    auto ObiLoader::init(InputObi input_obis, MallocFn malloc) -> void {
        DEBUG_ASSERT(malloc);
        malloc_ = malloc;

        auto first = input_obis.begin(), last = input_obis.end();
        for (; first != last; ++first) {
            if ((*first)->is_kernel()) {
                kernel_obi_ = first;
            }
        }

        DEBUG_ASSERT(kernel_obi_, "No bootable kernel found");
    }

    auto ObiLoader::load() -> void {

    }
}
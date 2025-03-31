#include <omitl/obi-header.hpp>

namespace omitl {
    auto validate_raw_obi_item_header(void *raw_header, usize const capacity) -> ustl::Result<ObiHeader *> {
        if (capacity < sizeof(ObiHeader)) {
            return ustl::err();
        }

        // TODO(SmallHuaZi) In near future, remove them to a single checking module.
        auto const header = reinterpret_cast<ObiHeader *>(raw_header);
        if (header->magic != OMI_HEADER_MAGIC) {
            return ustl::err();
        }
        if (header->length != capacity - sizeof(*header)) {
            return ustl::err();
        }
        if (header->flags & OMIF_CRC32) {
            // Check CRC32 code
        }

        return ustl::ok(header);
    }

} // namespace omitl
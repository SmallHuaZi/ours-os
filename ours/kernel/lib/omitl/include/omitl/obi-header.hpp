/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef OMI_OMI_HEADER_HPP
#define OMI_OMI_HEADER_HPP 1

#include <omitl/macro.hpp>
#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/result.hpp>
#include <ustl/mem/align.hpp>

namespace omitl {
    struct ObiHeader {
        FORCE_INLINE CXX11_CONSTEXPR
        static auto size() -> usize {
            return ustl::mem::align_up(sizeof(ObiHeader), OMI_ALIGNMENT);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto get_obi_payload_unchecked() -> u8 * {
            return reinterpret_cast<u8 *>(this) + size();
        } 

        u32 type;

        // Size of the payload immediately following this header. This
        // does not include the header itself nor any alignment padding
        // after the payload.
        u32 length;

        // Flags for this item.
        u32 flags;

        // Must be OMI_HEADER_MAGIC.
        u32 magic;

        // Must be the CRC32 of payload if ZBI_FLAGS_CRC32 is set,
        // otherwise must be ZBI_ITEM_NO_CRC32.
        u32 check_code;

        // Enable if flags was set HeaderFlags::Compressed.
        u32 extra;
    };
    static_assert(sizeof(ObiHeader) == OMI_SIZEOF_HEADER, "");

    FORCE_INLINE CXX11_CONSTEXPR
    static auto is_zipped(ObiHeader const &header) -> u32 {
        return header.flags & OMIF_ZIPPED;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto unzipped_size(ObiHeader const &header) -> u32 {
        return is_zipped(header) ? header.extra : header.length;
    }

    auto validate_raw_obi_item_header(void *header, usize const length) -> ustl::Result<ObiHeader *>;

    FORCE_INLINE CXX11_CONSTEXPR
    auto validate_raw_obi_container_header(void *raw_header, usize const length) -> ustl::Result<ObiHeader *> {
        auto header = reinterpret_cast<ObiHeader *>(raw_header);
        if (header->type != OMIT_CONTAINER) {
            return ustl::err();
        }

        return validate_raw_obi_item_header(header, length);
    }

} // namespace omitl

#endif // #ifndef OMI_OMI_HEADER_HPP
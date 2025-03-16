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
#ifndef OMI_OBI_HPP
#define OMI_OBI_HPP 1

#include <omi/defines.hpp>
#include <omi/header.hpp>

#include <ustl/function/fn.hpp>
#include <ustl/views/span.hpp>

namespace omi {
    struct Obi {
        auto validate() -> bool;

        auto is_kernel() -> bool {
            return header.type == OMIT_KERNEL;
        }

        auto is_zipped() -> bool {
            return header.flags & OMIF_ZIPPED;
        }

        auto unzipped_size() const -> usize {
            return header.extra;
        }

        auto size() const -> usize {
            return header.length;
        }

        auto data() const -> u8 const * {
            return payload;
        }

        Header header;
        u8 payload[];
    };

    struct ObiLoader {
        typedef ustl::views::Span<Obi *> InputObi;
        typedef ustl::function::Fn<auto(usize, usize) -> void *>    MallocFn;

        auto init(InputObi input_obis, MallocFn) -> void;

        auto load() -> void;

        auto load_one_obi(Obi const &obi) -> void;

        MallocFn malloc_;
        ustl::views::Span<Obi> input_;
        InputObi::IterMut kernel_obi_;
    };

} // namespace omi

#endif // #ifndef OMI_OBI_HPP
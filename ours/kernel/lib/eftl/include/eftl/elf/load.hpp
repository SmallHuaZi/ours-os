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
#ifndef EFTL_ELF_LOAD_HPP
#define EFTL_ELF_LOAD_HPP 1

#include <eftl/elf/segment.hpp>

#include <ours/types.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/limits.hpp>
#include <ustl/views/span.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace eftl::elf {
    /// This helper class is used to calculate the size of the image in memory.
    template <typename ElfTraits>
    struct VaSpaceObserver
        : public SegmentObserver<VaSpaceObserver<ElfTraits>, SegType::Load>
    {
        typedef typename ElfTraits::Addr    Addr;
        typedef typename ElfTraits::Segment       Segment;

        VaSpaceObserver(Addr ai_out &min, Addr ai_out &max) 
            : va_maximal_(ustl::mem::address_of(max)),
              va_minimal_(ustl::mem::address_of(min))
        {}

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe(Memory &&memory, Segment &segment, SegTypeMatcher<SegType::Load>) -> bool {
            *va_minimal_ = ustl::algorithms::min(*va_minimal_, segment.va_begin());
            *va_maximal_ = ustl::algorithms::max(*va_maximal_, segment.va_end());
            return true;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto finish() -> bool {
            return true;
        }

        Addr *va_maximal_;
        Addr *va_minimal_;
    };

    template <typename ElfTraits,
              template <typename> typename Collection>
    struct LoadInfoObserver;

    template <typename ElfTraits, 
              template <typename> typename Collection>
    struct LoadInfo {
        typedef typename ElfTraits::Addr    Addr;

        struct LoadableSegment {
            Addr va_start;
            Addr va_size;
            Addr alignment;
            SegPerms perms;
            ustl::views::Span<u8> content;
        };

        template <typename Visitor>
        auto visit_segments(Visitor &&visitor) -> bool {
            for (auto &segment : segments_) {
                if (!visitor(segment)) {
                    return false;
                }
            }

            return true;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto make_observer() -> LoadInfoObserver<ElfTraits, Collection> {
            return {*this};
        }

        Addr va_minimal_ = ustl::NumericLimits<Addr>::max();
        Addr va_maximal_ = 0;
        Collection<LoadableSegment> segments_;
    };

    template <typename ElfTraits,
              template <typename> typename Collection>
    struct LoadInfoObserver
        : public SegmentObserver<LoadInfoObserver<ElfTraits, Collection>, SegType::Load>
    {
        typedef typename ElfTraits::Segment       Segment;
        typedef LoadInfo<ElfTraits, Collection>   LoadInfo;

        FORCE_INLINE CXX11_CONSTEXPR
        LoadInfoObserver(LoadInfo &loadinfo)
            : loadinfo_(ustl::mem::address_of(loadinfo))
        {}

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe(Memory &&memory, Segment &segment, SegTypeMatcher<SegType::Load>) -> bool {
            if (segment.va_size() == 0) {
                return true;
            }

            loadinfo_->va_minimal_ = ustl::algorithms::min(loadinfo_->va_minimal_, segment.va_begin());
            loadinfo_->va_maximal_ = ustl::algorithms::max(loadinfo_->va_maximal_, segment.va_end());
            loadinfo_->segments_.emplace_back(
                segment.va_begin(), 
                segment.va_size(), 
                segment.alignment(),
                segment.permissions(), 
                ustl::views::make_span(memory.address() + segment.file_offset(), segment.file_size())
            );

            return true;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto finish() -> bool {
            return true;
        }

        LoadInfo *loadinfo_;
    };

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_LOAD_HPP
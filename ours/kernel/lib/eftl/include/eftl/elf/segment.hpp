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
#ifndef EFTL_ELF_SEGMENT_HPP
#define EFTL_ELF_SEGMENT_HPP 1

#include <eftl/elf/enums.hpp>
#include <eftl/elf/traits.hpp>
#include <eftl/elf/details/segment.hpp>

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/option.hpp>
#include <ustl/views/span.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/util/move.hpp>
#include <ustl/mem/address_of.hpp>

namespace eftl::elf {
    template <SegType>
    struct SegTypeMatcher {};

    template <typename Derived, SegType... Type>
    struct SegmentObserver {};

    template <typename Memory, typename Segment, typename Observer, SegType... Types>
    FORCE_INLINE CXX11_CONSTEXPR
    auto decode_segment(Memory &&memory, Segment &&segment, SegmentObserver<Observer, Types...> &observer) -> bool {
        auto is_ok = true;
        auto invoke_observe = [&] (auto const &tag) {
            is_ok = static_cast<Observer &>(observer).observe(memory, segment, tag);
            return true;
        };

        ((segment.type() == Types && invoke_observe(SegTypeMatcher<Types>())) || ...);
        return is_ok;
    }

    template <typename Memory, typename Segment, typename... Observer>
    auto decode_segments(Memory &&memory, ustl::views::Span<Segment> const &segments, Observer &&...observers) -> bool {
        for (auto &segment: segments) {
            if ((!decode_segment(memory, segment, observers) || ...)) {
                return false;
            }
        }

        return (observers.finish() && ...);
    }

    /// This observer class only watch a type of segment.
    template <typename ElfTraits, SegType Type>
    struct SegmentSingletonObserver
        : public SegmentObserver<SegmentSingletonObserver<ElfTraits, Type>, Type>
    {
        typedef typename ElfTraits::Segment      Segment;

        SegmentSingletonObserver(Segment &segment)
            : segment_(ustl::mem::address_of(segment))
        {}

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe(Memory &&memory, Segment &segment, SegTypeMatcher<Type>) -> bool {
            if (!segment_) {
                return false;
            }

            *segment_ = segment;
            return true;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto finish() -> bool {
            return true;
        }

        Segment *segment_;
    };

    template <typename ElfTraits>
    using LoadSegObserver = SegmentSingletonObserver<ElfTraits, SegType::Load>;

    template <typename ElfTraits>
    using DynSegObserver = SegmentSingletonObserver<ElfTraits, SegType::Dynamic>;

    template <typename ElfTraits>
    using TlsSegObserver = SegmentSingletonObserver<ElfTraits, SegType::Tls>;

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_SEGMENT_HPP
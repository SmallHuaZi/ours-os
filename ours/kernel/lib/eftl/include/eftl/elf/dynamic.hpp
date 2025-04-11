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
#ifndef EFTL_ELF_DYNAMIC_HPP
#define EFTL_ELF_DYNAMIC_HPP 1

#include <eftl/elf/details/dyn.hpp>

#include <ustl/array.hpp>
#include <ustl/limits.hpp>
#include <ustl/util/cktuple.hpp>
#include <ustl//util/type-map.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/views/span.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace eftl::elf {
    template <DynTag>
    struct DynTagMatcher {};

    template <typename Observer, DynTag... Tags>
    struct DynamicObserver {};

    template <typename Memory, typename Dyn, typename Observer, DynTag... Tags>
    auto decode_dynamic_at(Memory &&memory, Dyn &entry, DynamicObserver<Observer, Tags...>  &observer) -> bool {
        auto is_ok = true;
        auto invoke_observe = [&] (auto const &tag) {
            is_ok &= static_cast<Observer &>(observer).observe(memory, entry.d_val, tag);
            return true;
        };

        ((DynTag(entry.d_tag) == Tags && invoke_observe(DynTagMatcher<Tags>())) || ...);
        return is_ok;
    }

    template <typename Memory, typename Dyn, typename... Observer>
    auto decode_dynamic(Memory &&memory, ustl::views::Span<Dyn> &dynamics, Observer &&...observers) -> bool {
        for (auto &dyn: dynamics) {
            if (DynTag(dyn.d_tag) == DynTag::DtNull) {
                return (observers.finish(memory) && ...);
            }

            if ((!decode_dynamic_at(memory, dyn, observers) || ...)) {
                return false;
            }
        }
        return true;
    }

    template <typename Observer, typename Info, typename ElfTratis, typename TagToIndex, DynTag... Tags>
    struct DynInfoObserver: public DynamicObserver<Observer, Tags...> {
        typedef typename ElfTratis::Addr    Addr;
        typedef Info            InfoType;
        typedef TagToIndex      TagIndexMap;

        template <DynTag Tag>
        static usize const IndexV = ustl::TypeAlgos::Find<TagIndexMap, DynTagMatcher<Tag>>();

        CXX11_CONSTEXPR
        static auto const kMaxNumIndexies = ustl::NumericLimits<usize>::max();

        DynInfoObserver(Info &info)
            : info_(ustl::mem::address_of(info)),
              caches_()
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto info() -> InfoType & {
            return *info_;
        }

        template <typename Memory, DynTag Tag>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe(Memory &&memory, Addr val, DynTagMatcher<Tag>) -> bool {
            static_assert(!ustl::traits::IsSameV<ustl::TypeAlgos::Find<TagIndexMap, DynTagMatcher<Tag>>,
                                                 ustl::TypeAlgos::NotFound>, 
                                                 "Unsupported tag");
            caches_[IndexV<Tag>] = val;
            return true;
        }

        template <typename T, auto Setter, DynTag PosTag, DynTag SizeTag, DynTag CountTag = DynTag::Unknown, typename Memory>
        auto write_back(Memory &&memory) -> bool {
            Addr const pos = caches_[IndexV<PosTag>], size_bytes = caches_[IndexV<SizeTag>];
            // Incorrect address and/or length.
            if (!pos && !size_bytes) {
                return true;
            }
            if (!pos) {
                return false;
            } else if (!size_bytes) {
                return false;
            }

            // Misalignment.
            if ((pos % alignof(T)) != 0) {
                return false;
            }
            if ((size_bytes % alignof(T)) != 0) {
                return false;
            }

            auto table = memory.template read_at_n<T>(pos, size_bytes / sizeof(T));
            if CXX11_CONSTEXPR (CountTag != DynTag::Unknown) {
                Addr const count = caches_[IndexV<CountTag>];
                // Incorrect length.
                if (count > size_bytes / sizeof(T)) {
                    return false;
                }
                (info_->*Setter)(table, count);
            } else {
                (info_->*Setter)(table);
            }

            return true;
        }

        template <DynTag Tag>
        FORCE_INLINE CXX11_CONSTEXPR
        auto at() -> Addr {
            return caches_[IndexV<Tag>];
        }
    private:
        Info *info_;
        ustl::Array<Addr, sizeof...(Tags)> caches_;
    };

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_DYNAMIC_HPP
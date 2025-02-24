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

#ifndef USTL_UTIL_ENUMSEQUENCE_HPP
#define USTL_UTIL_ENUMSEQUENCE_HPP 1

namespace ustl {
    template <typename Enum, Enum ...>
    struct EnumSequence
    {};

    template <typename Enum, int Min, int Max, int... Next>
    struct MakeEnumSequence 
        : public MakeEnumSequence<Enum, Min, Max - 1, Max - 1, Next...>
    {};

    template <typename Enum, int Min, int ... Next>
    struct MakeEnumSequence<Enum, Min, Min, Next ... >
    { using Type = EnumSequence<Enum, static_cast<Enum>(Next) ... >; };

    template <typename Enum, int Min, int Max>
    using MakeEnumSequenceT = typename MakeEnumSequence<Enum, Min, Max>::Type;

} // namespace ustl::util

#endif // #ifndef USTL_UTIL_ENUMSEQUENCE_HPP
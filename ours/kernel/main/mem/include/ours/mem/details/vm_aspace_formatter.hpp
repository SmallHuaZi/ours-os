// ours/mem OURS/MEM_VM_ASPACE_FORMATTER_HPP
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

#ifndef OURS_MEM_VM_ASPACE_FORMATTER_HPP
#define OURS_MEM_VM_ASPACE_FORMATTER_HPP 1

#include <ours/mem/types.hpp>
#include <ustl/fmt/formatter.hpp>

template <typename Char>
class ustl::fmt::Formatter<ours::mem::VmAspace, Char>
{
    typedef Formatter           Self;
    typedef ours::mem::VmAspace Object;

public:
    template <typename ParseContext>
    auto parse(ParseContext &) const -> ParseContext::IterMut;

    template <typename FormatContext>
    auto format(Object const &, FormatContext &) const -> FormatContext::IterMut;

    template <typename FormatContext>
    auto formatted_size(Object const &, FormatContext &) const -> usize;
};

#endif // #ifndef OURS_MEM_VM_ASPACE_FORMATTER_HPP
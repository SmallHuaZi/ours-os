#include <ours/mem/vm_aspace_formatter.hpp>
#include <ustl/fmt/format.hpp>
#include <ustl/fmt/details/fwd.hpp>
#include <ustl/fmt/details/basic_format_parse_context.hpp>

using ours::mem::VmAspace;
using ustl::fmt::Formatter;
using ustl::fmt::FormatParseContext;

template <typename Char>
template <typename ParseContext>
auto Formatter<VmAspace, Char>::parse(ParseContext &) const 
    -> typename ParseContext::IterMut
{}

template
auto Formatter<VmAspace, char>::parse(FormatParseContext &) const 
    -> FormatParseContstl::IterMut;

template <typename Char>
template <typename FormatContext>
auto Formatter<VmAspace, Char>::format(Object const &, FormatContext &) const 
    -> typename FormatContext::IterMut
{}
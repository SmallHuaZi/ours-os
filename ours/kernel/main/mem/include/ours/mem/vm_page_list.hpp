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
#ifndef OURS_MEM_VM_PAGE_LIST_HPP
#define OURS_MEM_VM_PAGE_LIST_HPP 1

#include <ours/const.hpp>
#include <ours/mem/vm_page.hpp>

#include <gktl/xarray.hpp>
#include <ustl/bitfields.hpp>

namespace ours::mem {
    struct PageOrMarker {
        enum class Type {
            None,
            Page,
            PageRef,
            MaxNumType,
        };
        template <usize Id, typename Type, usize Bits, bool Natural = false>
        using Field = ustl::Field<ustl::bitfields::Id<Id>, 
                                  ustl::bitfields::Type<Type>, 
                                  ustl::bitfields::Bits<Bits>,
                                  ustl::bitfields::Natural<Natural>>;
        using Inner = ustl::BitFields<Field<0, Type, ustl::bit_width(usize(Type::MaxNumType))>,
                                      Field<1, VmPage *, kMaxVirtAddrBit - ustl::bit_width(usize(Type::MaxNumType))>>;

        Inner inner;
    };

    class VmPageList {

    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_LIST_HPP
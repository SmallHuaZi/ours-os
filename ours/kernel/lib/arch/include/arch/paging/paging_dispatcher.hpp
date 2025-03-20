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
#ifndef ARCH_PAGING_PAGING_DISPATCHER_HPP
#define ARCH_PAGING_PAGING_DISPATCHER_HPP 1

#include <arch/types.hpp>

namespace arch::paging {
    /// Parameterizes the unaddressable bits of a virtual address.
    enum class VirtAddrExt {
        /// Sign-extended (commonly referred to as "canonical"): each unaddressable
        /// bit must match the highest addressable bit.
        ///
        /// Conventionally used in paging schemes that configure the upper and lower
        /// spaces together: the subspace that is sign-extended as 0 represents the
        /// lower, while the subspace that is sign-extended as 1 represents the upper.
        Canonical,

        /// All unaddressable bits are 0.
        ///
        /// Conventionally used for describing the lower address space in paging
        /// schemes that configure the upper and lower spaces separately.
        AlwaysZero,

        /// All unaddressable bits are 1.
        ///
        /// Conventionally used for describing the upper address space in paging
        /// schemes that configure the upper and lower spaces separately.
        AlwaysOne,
    };

    /// Interface class for the specilizations to `Paging` variants on an architecture.
    ///
    /// Example:
    ///     struct X86PageExt; // This is a option to enable page extension on 32-bits platform.
    ///     struct X86ExtPaging {...};
    ///     template <> PagingDispatcher<3, X86PageExt> { typedef X86ExtPaging Type;  };
    ///     
    template <usize PagingLevel, typename... Options>
    struct PagingDispatcher;

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_PAGING_DISPATCHER_HPP
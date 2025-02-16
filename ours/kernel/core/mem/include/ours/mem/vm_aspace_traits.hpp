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

#ifndef OURS_MEM_ARCH_VM_ASPACE_TRAITS_HPP
#define OURS_MEM_ARCH_VM_ASPACE_TRAITS_HPP 1

#include <ours/status.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/mmu_flags.hpp>

#include <ustl/util/enum_bits.hpp>
#include <ustl/traits/function_detector.hpp>

namespace ours::mem {
    enum class VmasFlags: usize {
        User,
        Kernel,
        Guest,
        OomIgnored = BIT(2),
        AreaMergeable = BIT(3),
        Shared = BIT(4),
    };
    USTL_ENABLE_ENUM_BITS(VmasFlags);

    enum class HarvestAction: usize {
        /// Only updates the frame's age if it is accessed.
        UpdateAge = BIT(0),

        /// If the entry is unaccessed then we zap it. 
        ZapUnaccessed = BIT(1),

        /// If the frame is accessed we remove the accessed flag.
        ResetUnaccessed = BIT(2),
    };
    USTL_ENABLE_ENUM_BITS(HarvestAction);

    /// `ArchVmAspaceTraits` is a trait class that defines the required interfaces for `ArchVmAspace`.
    /// It serves as a compile-time contract, ensuring that all architectures implement the necessary
    /// functionality for `VmAspace`.
    /// 
    /// Reasons for choosing static traits over runtime polymorphism:
    /// 1. Virtual functions are typically used to support polymorphism and maintain interface consistency,
    ///    but they inevitably introduce runtime overhead. In this case, since we only need to enforce
    ///    interface consistency, static traits are a more efficient alternative to virtual functions.
    /// 2. While some architectures (e.g., x86 with EPT) may require runtime polymorphism to handle
    ///    environment-specific requirements, this behavior is architecture-specific and cannot be
    ///    assumed universally. Minimizing performance degradation is a core goal of kernel development,
    ///    and static traits help achieve this by avoiding unnecessary runtime costs.
    template <typename ArchVmAspace>
    class ArchVmAspaceTraits
    {
        USTL_MPL_CREATE_METHOD_DETECTOR(init, Init);
        static auto init(ArchVmAspace &aspace) -> Status
        {
            static_assert(HasFnInit<auto (ArchVmAspace::*)() -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::init`");
            return aspace.init();
        }

        USTL_MPL_CREATE_METHOD_DETECTOR(map, Map);
        static auto map(ArchVmAspace &aspace, VirtAddr va, usize n, MmuFlags flags) -> Status
        {
            static_assert(HasFnMap<auto (ArchVmAspace::*)(VirtAddr, usize, MmuFlags) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::map`");
            return aspace.map(va, n, flags);
        }
        
        USTL_MPL_CREATE_METHOD_DETECTOR(map_to, MapTo);
        static auto map_to(ArchVmAspace &aspace, PhysAddr pa, VirtAddr va, usize n, MmuFlags flags) -> Status
        {
            static_assert(HasFnMapTo<auto (ArchVmAspace::*)(PhysAddr, VirtAddr, usize, MmuFlags) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::map_to`");
            return aspace.map_to(pa, va, n, flags);
        }
        
        USTL_MPL_CREATE_METHOD_DETECTOR(unmap, Unmap);
        static auto unmap(ArchVmAspace &aspace, VirtAddr va, usize n) -> Status
        {
            static_assert(HasFnMapTo<auto (ArchVmAspace::*)(VirtAddr, usize) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::map_to`");
            return aspace.unmap(va, n);
        }

        USTL_MPL_CREATE_METHOD_DETECTOR(protect, Protect);
        static auto protect(ArchVmAspace &aspace, VirtAddr va, usize n, MmuFlags flags) -> Status
        {
            static_assert(HasFnProtect<auto (ArchVmAspace::*)(VirtAddr, usize, MmuFlags) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::protect`");
            return aspace.protect(va, n, flags);
        }

        USTL_MPL_CREATE_METHOD_DETECTOR(query, Query);
        static auto query(ArchVmAspace &aspace, VirtAddr va, ai_out PhysAddr *pa, ai_out MmuFlags *flags) -> Status
        {
            static_assert(HasFnProtect<auto (ArchVmAspace::*)(VirtAddr, PhysAddr *, MmuFlags *) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::query`");
            return aspace.query(va, pa, flags);
        }

        USTL_MPL_CREATE_METHOD_DETECTOR(mark_accessed, MarkAccessed);
        static auto mark_accessed(ArchVmAspace &aspace, VirtAddr va, usize n) -> Status
        {
            static_assert(HasFnMarkAccessed<auto (ArchVmAspace::*)(VirtAddr, usize) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::mark_accessed`");
            return aspace.harvest_accessed(va, n);
        }

        USTL_MPL_CREATE_METHOD_DETECTOR(harvest_accessed, HarvestAccessed);
        static auto harvest_accessed(ArchVmAspace &aspace, VirtAddr va, usize n, HarvestAction action) -> Status
        {
            static_assert(HasFnMarkAccessed<auto (ArchVmAspace::*)(VirtAddr, usize, HarvestAction) -> Status>::VALUE,
            "The class do not implements method `ArchVmAspace::harvest_accessed`");
            return aspace.harvest_accessed(va, n, action);
        }
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_ARCH_VM_ASPACE_TRAITS_HPP
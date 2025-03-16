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

#include <ustl/result.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/traits/function_detector.hpp>

#include <arch/paging/controls.hpp>

namespace ours::mem {
    using arch::paging::MapControl;
    using arch::paging::UnMapControl;
    using arch::paging::HarvestControl;

    enum class VmasFlags: usize {
        User,
        Kernel,
        Guest,
        OomIgnored = BIT(2),
        AreaMergeable = BIT(3),
        Shared = BIT(4),
        Active = BIT(5),
    };
    USTL_ENABLE_ENUM_BITS(VmasFlags);

    /// `ArchVmAspaceConstraints` is a trait class that asserts the required interfaces has been implemneted
    /// by `ArchVmAspace`. It serves as a compile-time checker, ensuring that all architectures implement the 
    /// necessary functionality for `VmAspace`.
    /// 
    /// Reasons for choosing static traits check over virtual function:
    /// 1. Virtual functions are typically used to support polymorphism and maintain interface consistency,
    ///    but they inevitably introduce runtime overhead. In this case, since we only need to enforce
    ///    interface consistency, static traits are a more efficient alternative to virtual functions.
    /// 2. While some architectures (e.g., x86 with EPT) may require runtime polymorphism to handle
    ///    environment-specific requirements, this behavior is architecture-specific and cannot be
    ///    assumed universally. Minimizing performance degradation is a core goal of kernel development,
    ///    and static traits help achieve this by avoiding unnecessary runtime costs.
    template <typename ArchVmAspace>
    class ArchVmAspaceConstraints
        : public ustl::traits::TrueType
    {
        USTL_MPL_CREATE_METHOD_DETECTOR(init, Init);
        static_assert(HasFnInit<auto (ArchVmAspace::*)() -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::init`");
        
        USTL_MPL_CREATE_METHOD_DETECTOR(map, Map);
        static_assert(HasFnMap<auto (ArchVmAspace::*)(VirtAddr, PhysAddr, usize, MmuFlags, MapControl) -> ustl::Result<usize, Status>>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::map`");

        USTL_MPL_CREATE_METHOD_DETECTOR(map_bulk, MapBulk);
        static_assert(HasFnMapBulk<auto (ArchVmAspace::*)(VirtAddr, PhysAddr *, usize, MmuFlags, MapControl) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::map_bulk`");
        
        USTL_MPL_CREATE_METHOD_DETECTOR(unmap, Unmap);
        static_assert(HasFnUnmap<auto (ArchVmAspace::*)(VirtAddr, usize, UnMapControl) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::unmap`");

        USTL_MPL_CREATE_METHOD_DETECTOR(protect, Protect);
        static_assert(HasFnProtect<auto (ArchVmAspace::*)(VirtAddr, usize, MmuFlags) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::protect`");

        USTL_MPL_CREATE_METHOD_DETECTOR(query, Query);
        static_assert(HasFnQuery<auto (ArchVmAspace::*)(VirtAddr, PhysAddr *, MmuFlags *) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::query`");

        USTL_MPL_CREATE_METHOD_DETECTOR(mark_accessed, MarkAccessed);
        static_assert(HasFnMarkAccessed<auto (ArchVmAspace::*)(VirtAddr, usize) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::mark_accessed`");

        USTL_MPL_CREATE_METHOD_DETECTOR(harvest_accessed, HarvestAccessed);
        static_assert(HasFnHarvestAccessed<auto (ArchVmAspace::*)(VirtAddr, usize, HarvestControl) -> Status>::VALUE,
        "The ArchVmAspace do not implements required method `ArchVmAspace::harvest_accessed`");
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_ARCH_VM_ASPACE_TRAITS_HPP
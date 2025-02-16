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

#ifndef GKTL_CPULOCAL_HPP
#define GKTL_CPULOCAL_HPP 1

#include <ours/cpu.hpp>
#include <ours/cpu_mask.hpp>
#include <ours/assert.hpp>

#include <ustl/function/invoke.hpp>
#include <ustl/collections/array.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

#ifndef CPU_LOCAL 
#   define CPU_LOCAL LINK_SECTION(".kernel.cpu_local")
#endif

namespace ours {
    /// Define a CPU-local variable like this:
    /// CPU_LOCAL [static] YourType VAR_NAME;
    ///
    /// To access it via using:
    /// CpuLocal::access[<YourType>](&VAR_NAME);
    ///
    /// To hide the name of a variable as a global symbol or perform permission checks,
    /// specialize `access` like this:
    /// template <>
    /// auto CpuLocal::access<YourType>() -> YourType * {
    ///     CPU_LOCAL static YourType VAR_NAME;
    ///     // Do check
    ///     if (/* Check failure */) {
    ///         return CpuLocal::access(&VAR_NAME);
    ///     }
    ///     return 0;
    /// }
    ///
    /// Then access it with:
    /// CpuLocal::access<YourType>();
    ///
    class CpuLocal
    {
        typedef CpuLocal    Self;
    public:
        static auto init(CpuId cpuid) -> Status;

        static auto cpuid() -> CpuId
        { return arch_current_cpu(); }

        // The `default` object should be accessed by this interface.
        template <typename T, typename = T>
        static auto access(CpuId = Self::cpuid()) -> T *;

        // The `arch-custom` object should be accessed by this interface.
        template <typename T, typename = T>
        static auto access(T *object, CpuId = Self::cpuid()) -> T *;

        template <typename T, typename F>
        static auto for_each(F &&f) -> void;

        template <typename T, typename F>
        static auto for_each(T *object, F &&f) -> void;

    private:
        static char CPU_LOCAL_START[] LINK_NAME("KERNEL_CPU_LOCAL_START");
        static char CPU_LOCAL_END[] LINK_NAME("KERNEL_CPU_LOCAL_END");
        static ustl::collections::Array<isize, MAX_CPU_NUM> CPU_LOCAL_OFFSET;
    };

    template <typename T, typename>
    FORCE_INLINE
    auto CpuLocal::access(T *object, CpuId cpuid) -> T *
    {
        auto addr = reinterpret_cast<char *>(object);
        return reinterpret_cast<T *>(Self::CPU_LOCAL_OFFSET[cpuid._0] + addr);
    }

    /// Invoker prototype of `F` like --void f(T *)-- 
    template <typename T, typename F>
    FORCE_INLINE
    auto CpuLocal::for_each(T *object, F &&f) -> void
    {
        for_each_possible_cpu([object, f] (CpuId cpuid) {
            auto local_object = Self::access(object, cpuid);
            ustl::function::invoke(f, local_object);
        });
    }

} // namespace ours 

#endif // #ifndef GKTL_CPULOCAL_HPP
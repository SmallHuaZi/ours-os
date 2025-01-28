// gktl GKTL_CPULOCAL_HPP
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

#include <ours/types.hpp>
#include <ours/status.hpp>
#include <ours/config.hpp>
#include <ours/assert.hpp>
#include <ours/cpu.hpp>

#include <ustl/views/span.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/collections/array.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

#ifndef CPU_LOCAL 
#   define CPU_LOCAL LINK_SECTION(".kernel.cpu_local")
#endif

namespace gktl {
    using ours::CpuId;
    using ours::Status;

    /// Define a CPU-local variable like this:
    /// CPU_LOCAL [static] YourType VAR_NAME;
    ///
    /// Access it using:
    /// CpuLocal::access[<YourType>](&VAR_NAME);
    ///
    /// To hide the name of a variable as a global symbol or perform permission checks,
    /// specialize `access` like this:
    /// template <>
    /// auto CpuLocal::access<YourType>() -> YourType * {
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

        static auto cpuid() -> CpuId;

        // The `default` object should be accessed by this interface.
        template <typename T>
        static auto access(CpuId = Self::cpuid()) -> T *;

        // The `arch-custom` object should be accessed by this interface.
        template <typename T>
        static auto access(T *object, CpuId = Self::cpuid()) -> T *;

        template <typename T, typename F>
        static auto for_each(F f) -> void;

        template <typename T, typename F>
        static auto for_each(T *object, F f) -> void;

    private:
        static char CPU_LOCAL_START[] LINK_NAME("KERNEL_CPU_LOCAL_START");
        static char CPU_LOCAL_END[] LINK_NAME("KERNEL_CPU_LOCAL_END");
        static ustl::collections::Array<char *, ours::MAX_CPU_NUM> CPU_LOCAL_MAP;
    };

    inline auto CpuLocal::cpuid() -> ours::CpuId
    {
        CPU_LOCAL static ours::CpuId CPU_ID;
        return *CpuLocal::access(&CPU_ID); 
    }

    template <typename T>
    inline auto CpuLocal::access(T *object, ours::CpuId cpuid) -> T *
    {
        auto addr = reinterpret_cast<char *>(object);
        DEBUG_ASSERT(cpuid < Self::CPU_LOCAL_MAP.size())
        DEBUG_ASSERT(Self::CPU_LOCAL_START <= addr);
        DEBUG_ASSERT(addr + sizeof(T) < Self::CPU_LOCAL_END);

        auto offset = addr - Self::CPU_LOCAL_START;
        return reinterpret_cast<T *>(Self::CPU_LOCAL_MAP[cpuid._0] + offset);
    }

    /// Invoker prototype of `F` like --void f(T &)-- 
    template <typename T, typename F>
    inline auto CpuLocal::for_each(T *object, F f) -> void
    {
        auto addr = reinterpret_cast<char *>(object);
        DEBUG_ASSERT(cpuid < Self::CPU_LOCAL_MAP.size())
        DEBUG_ASSERT(Self::CPU_LOCAL_START <= addr);
        DEBUG_ASSERT(addr + sizeof(T) < Self::CPU_LOCAL_END);

        auto offset = addr - Self::CPU_LOCAL_START;
        for (auto start : Self::CPU_LOCAL_MAP) {
            auto obj = (start + offset);
            ustl::function::invoke(f, *obj);
        }
    }

} // namespace ours 

#endif // #ifndef GKTL_CPULOCAL_HPP
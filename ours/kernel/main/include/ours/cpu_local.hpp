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
#include <ours/init.hpp>
#include <ours/assert.hpp>
#include <ours/mem/cfg.hpp>

#include <ustl/function/invoke.hpp>
#include <ustl/collections/array.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

#ifndef CPU_LOCAL 
#   define CPU_LOCAL LINK_SECTION(".kernel.cpu_local")
#endif

namespace ours {
    template <typename T>
    struct PerCpu
    {
        template <typename F>
            requires ustl::traits::Invocable<F, T &>
        auto with_current(F &&f)
        {}

        T *value_;
    };

    /// Define a static lifetime CPU-local variable like this:
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
        INIT_CODE
        static auto init(u32 nr_cpu) -> Status;

        static auto cpu_num() -> CpuNum
        { return arch_current_cpu(); }

        // The `default` object should be accessed by this interface.
        template <typename T, typename = T>
        static auto access(CpuNum = Self::cpu_num()) -> T *;

        // The `arch-custom` object should be accessed by this interface.
        template <typename T, typename = T>
        static auto access(T *object, CpuNum cpunum = Self::cpu_num()) -> T *;

        template <typename T, typename = T>
        static auto allocate(CpuNum cpunum) -> PerCpu<T>;

        template <typename T, typename = T>
        static auto free(PerCpu<T>, CpuNum cpunum) -> void;

        template <typename T, typename F>
        static auto for_each(F &&f) -> void;

        template <typename T, typename F>
        static auto for_each(T *object, F &&f) -> void;
    private:
        static ustl::collections::Array<isize, MAX_CPU_NUM>     UNIT_OFFSET;
        static ustl::collections::Array<isize, MAX_NODES>  GROUP_OFFSET;
    };

    template <typename T, typename>
    FORCE_INLINE
    auto CpuLocal::access(T *object, CpuNum cpunum) -> T *
    {
        auto addr = reinterpret_cast<char *>(object);
        return reinterpret_cast<T *>(Self::UNIT_OFFSET[cpunum] + addr);
    }

    /// Invoker prototype of `F` like --void f(T *)-- 
    template <typename T, typename F>
    FORCE_INLINE
    auto CpuLocal::for_each(T *object, F &&f) -> void
    {
        for_each_possible_cpu([object, f] (CpuNum cpunum) {
            auto local_object = Self::access(object, cpunum);
            ustl::function::invoke(f, local_object);
        });
    }

} // namespace ours 

#endif // #ifndef GKTL_CPULOCAL_HPP
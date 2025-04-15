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

#if !__has_include(<ours/arch/cpu-local.hpp>)
#   error "Architecure do not provide the header `ours/arch/cpu-local.hpp`"
#endif

#include <ours/arch/cpu-local.hpp>

#include <ours/cpu.hpp>
#include <ours/cpu-mask.hpp>
#include <ours/init.hpp>
#include <ours/assert.hpp>
#include <ours/mem/cfg.hpp>

#include <ustl/array.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/traits/integral.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/traits/invoke_result.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

#ifndef CPU_LOCAL
#   define CPU_LOCAL LINK_SECTION(".init.data.cpu_local") FORCE_USED
#endif

namespace ours {
    template <typename T>
    struct PerCpu {
        template <typename F>
            requires ustl::traits::Invocable<F, T &>
        auto with_current(F &&f) -> ustl::traits::InvokeResultT<F, T &>
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
    /// Something to note:
    ///     i)  Do not apply self-reference on a cpu local variable.
    ///     ii) Do not save the address of a cpu local variable during boot-time.
    ///
    class CpuLocal {
        typedef CpuLocal    Self;
    public:
        INIT_CODE
        static auto init_early() -> void {
            arch_install(0);
        }

        INIT_CODE
        static auto init(CpuMask const &cpus) -> Status;

        INIT_CODE FORCE_INLINE
        static auto init_percpu() -> void {
            auto const thiscpu = arch_current_cpu();
            arch_install(s_cpu_offset[thiscpu]);
            write(s_current_cpu_offset, s_cpu_offset[thiscpu]);
        }

        template <typename Integral>
        FORCE_INLINE
        static auto read(Integral &integer) -> Integral {
            static_assert(ustl::traits::IsIntegralV<Integral>);
            static_assert(sizeof(usize) == sizeof(&integer), "`usize` must has the same size with a pointer");
            return arch_cpu_local_read<Integral>(reinterpret_cast<usize>(&integer));
        }

        template <typename Integral>
        FORCE_INLINE
        static auto write(Integral &integer, Integral value) -> void {
            static_assert(ustl::traits::IsIntegralV<Integral>);
            arch_cpu_local_write(reinterpret_cast<usize>(&integer), value);
        }

        // Reserved interface for hiding global symbols in certain cases.
        template <typename T>
        static auto access() -> T *;

        // Reserved interface for hiding global symbols in certain cases.
        template <typename T>
        static auto access(CpuNum cpunum) -> T *;

        // The `arch-custom` object should be accessed by this interface.
        template <typename T>
        FORCE_INLINE
        static auto access(T *object) -> T * {
            auto const ptr = reinterpret_cast<T *>(
                reinterpret_cast<u8 *>(object) + read(s_current_cpu_offset)
            );
            DEBUG_ASSERT(ustl::mem::is_aligned(ptr, alignof(T)), "Invalid cpu local data pointer");

            return ptr;
        }

        // The `arch-custom` object should be accessed by this interface.
        template <typename T>
        FORCE_INLINE
        static auto access(T *object, CpuNum cpunum) -> T * {
            DEBUG_ASSERT(cpunum < MAX_CPU_NUM, "");
            auto const ptr = reinterpret_cast<T *>(
                reinterpret_cast<u8 *>(object) + s_cpu_offset[cpunum]
            );
            DEBUG_ASSERT(ustl::mem::is_aligned(ptr, alignof(T)), "Invalid cpu local data pointer");

            return ptr;
        }

        template <typename T>
        static auto allocate() -> PerCpu<T>;

        template <typename T>
        static auto free(PerCpu<T>) -> void;

        template <typename T, typename F>
        static auto for_each(F &&f) -> void {
            for_each_possible_cpu([f] (CpuNum cpunum) {
                auto local_object = Self::access<T>(cpunum);
                ustl::function::invoke(f, local_object);
            });
        }

        template <typename T, typename F>
        FORCE_INLINE
        static auto for_each(T *object, F &&f) -> void {
            for_each_possible_cpu([object, f] (CpuNum cpunum) {
                auto local_object = Self::access(object, cpunum);
                ustl::function::invoke(f, local_object);
            });
        }
    private:
        FORCE_INLINE
        static auto arch_install(usize offset) -> void {
            arch_cpu_local_install(offset);
        }

        /// This can save a time of indirect addressing.
        CPU_LOCAL
        static inline isize s_current_cpu_offset = 0;
        static inline ustl::Array<isize, MAX_CPU_NUM> s_cpu_offset;
        static inline ustl::Array<isize, MAX_NODES>   s_group_offset;
    };

} // namespace ours

#endif // #ifndef GKTL_CPULOCAL_HPP
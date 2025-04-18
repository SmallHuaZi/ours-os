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
#include <ours/mem/types.hpp>
#include <ours/mem/gaf.hpp>

#include <ustl/array.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/traits/integral.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/traits/invoke_result.hpp>
#include <ustl/algorithms/minmax.hpp>

#ifndef DEBUG_ASSERT
#   define DEBUG_ASSERT(...)
#endif

#ifndef CPU_LOCAL
#   define CPU_LOCAL LINK_SECTION(".init.data.cpu_local") FORCE_USED
#endif

namespace ours {
    template <typename T>
    struct PerCpu;

    template <typename T>
    PerCpu(T *) -> PerCpu<T>;

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
        static auto init_early() -> void;

        INIT_CODE FORCE_INLINE
        static auto init() -> Status {
            return init(ArchCpuLocal::kDynFirstChunkSize, ArchCpuLocal::kUnitAlign);
        }

        INIT_CODE FORCE_INLINE
        static auto init_percpu() -> void {
            auto const thiscpu = arch_current_cpu();
            install(s_cpu_offset[thiscpu]);
            write(s_current_cpu_offset, s_cpu_offset[thiscpu]);
        }

        static auto dump() -> void;

        template <typename Integral>
        FORCE_INLINE
        static auto read(Integral &integer) -> Integral {
            static_assert(ustl::traits::IsIntegralV<Integral>);
            static_assert(sizeof(usize) == sizeof(&integer), "`usize` must has the same size with a pointer");
            return ArchCpuLocal::read<Integral>(reinterpret_cast<usize>(&integer));
        }

        template <typename Integral>
        FORCE_INLINE
        static auto write(Integral &integer, Integral value) -> void {
            static_assert(ustl::traits::IsIntegralV<Integral>);
            ArchCpuLocal::write(reinterpret_cast<usize>(&integer), value);
        }

        /// Reserved interface for hiding global symbols in certain cases.
        template <typename T>
        static auto access() -> T *;

        /// Reserved interface for hiding global symbols in certain cases.
        template <typename T>
        static auto access(CpuNum cpunum) -> T *;

        template <typename T>
        FORCE_INLINE
        static auto access(T *object) -> T * {
            auto const ptr = reinterpret_cast<T *>(
                reinterpret_cast<u8 *>(object) + read(s_current_cpu_offset)
            );
            DEBUG_ASSERT(ustl::mem::is_aligned(ptr, alignof(T)), "Invalid cpu local data pointer");

            return ptr;
        }

        template <typename T>
        FORCE_INLINE
        static auto access(T *object, CpuNum cpunum) -> T * {
            DEBUG_ASSERT(cpunum < MAX_CPU, "");
            auto const ptr = reinterpret_cast<T *>(
                reinterpret_cast<u8 *>(object) + s_cpu_offset[cpunum]
            );
            DEBUG_ASSERT(ustl::mem::is_aligned(ptr, alignof(T)), "Invalid cpu local data pointer");

            return ptr;
        }

        template <typename T>
        FORCE_INLINE
        static auto allocate(AlignVal align= alignof(T), mem::Gaf gaf = mem::kGafKernel) -> PerCpu<T> {
            return PerCpu(reinterpret_cast<T *>(Self::allocate(sizeof(T), align, gaf))); 
        }

        template <typename T>
        FORCE_INLINE
        static auto free(PerCpu<T> object) -> void {
            return free(object.object);
        }

        template <typename T, typename F>
        FORCE_INLINE
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
        static auto init(usize dyn_size, usize unit_align) -> Status;
        static auto allocate(usize size, AlignVal val, mem::Gaf gaf) -> void *;
        static auto free(void *) -> void;

        FORCE_INLINE
        static auto install(usize offset) -> void {
            ArchCpuLocal::install(offset);
        }

        /// This can save a time of indirect addressing.
        CPU_LOCAL
        static inline isize s_current_cpu_offset = 0;
        static inline ustl::Array<isize, MAX_CPU>   s_cpu_offset;
    };

    /// A wrapper to prevent directly using the pointer.
    template <typename T>
    struct PerCpu {
        friend CpuLocal;

        PerCpu() = default;

        FORCE_INLINE CXX11_CONSTEXPR
        explicit PerCpu(T *object)
            : object_(object)
        {}

        ~PerCpu();

        template <typename F>
            requires ustl::traits::Invocable<F, T &>
        auto with_current(F &&f) -> ustl::traits::InvokeResultT<F, T &> {
            return ustl::function::invoke(f, *CpuLocal::access(object_));
        }

        template <typename F>
            requires ustl::traits::Invocable<F, T const &>
        auto with_current(F &&f) const -> ustl::traits::InvokeResultT<F, T const &> {
            return ustl::function::invoke(f, *CpuLocal::access(object_));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        operator bool() {
            return object_ != nullptr;
        }

    private:
        T *object_;
    };

} // namespace ours

#endif // #ifndef GKTL_CPULOCAL_HPP
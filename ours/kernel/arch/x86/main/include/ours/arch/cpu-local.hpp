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
#ifndef OURS_ARCH_CPU_LOCAL_HPP
#define OURS_ARCH_CPU_LOCAL_HPP 1

#include <ours/mem/cfg.hpp>

#include <arch/x86/msr.hpp>
#include <arch/x86/fsgs.hpp>

#include <ustl/traits/ptr.hpp>
#include <ustl/traits/integral.hpp>

namespace ours {
    struct ArchCpuLocal {
        CXX11_CONSTEXPR
        static usize const kDynFirstChunkSize = KB(16);

        CXX11_CONSTEXPR
        static usize const kUnitAlign = PAGE_SIZE;

        template <typename T>
        FORCE_INLINE
        static auto install(T base) -> void {
            static_assert(ustl::traits::IsIntegralV<T> || ustl::traits::IsPtrV<T>);

            using namespace arch;
            MsrIo::write(MsrRegAddr::IA32GsBase, base);
        }

        template <typename Integer>
        FORCE_INLINE
        static auto read(usize offset) -> Integer {
            return arch::read_gs_offset<Integer>(offset);
        }

        template <typename Integer>
        FORCE_INLINE
        static auto write(usize offset, Integer value) -> void {
            arch::write_gs_offset<Integer>(offset, value);
        }
    };

} // namespace ours::arch

#endif // #ifndef OURS_ARCH_CPU_LOCAL_HPP
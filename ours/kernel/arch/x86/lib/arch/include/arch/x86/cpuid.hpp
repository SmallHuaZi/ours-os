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
#ifndef ARCH_X86_CPUID_HPP
#define ARCH_X86_CPUID_HPP 1

#include <arch/x86/cpuid-leaf.hpp>
#include <arch/processor/cpu-states.hpp>

namespace arch {
    struct CpuId {
        typedef CpuId   Self;
        CpuId() = default;

        CpuId(u32 eax, u32 ebx, u32 ecx, u32 edx) 
            : eax_(eax), ebx_(ebx), ecx_(ecx), edx_(edx)
        {}

        ~CpuId() = default;

        FORCE_INLINE
        static auto has() -> bool {
            return ArchCpuState::read()
                                .set<ArchCpuState::CpuId>(1)
                                .write()
                                .get<ArchCpuState::CpuId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto query(CpuIdLeaf leaf, CpuIdSubLeaf subleaf = CpuIdSubLeaf(0)) -> void {
            DEBUG_ASSERT(has(), "Use cpuid on a unsupported platform");
    	    asm volatile("cpuid"
               : "=a" (eax_), "=b" (ebx_), "=c" (ecx_), "=d" (edx_)
    	        : "a" (leaf), "c" (subleaf)
    	        : "memory"
            );
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto query_safe(CpuIdLeaf leaf, CpuIdSubLeaf subleaf = CpuIdSubLeaf(0)) -> bool {
            if (has()) {
                query(leaf, subleaf);
                return true;
            }

            return false;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_eax(u32 eax) -> Self & {
            eax_ = eax;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto eax() -> u32 {
            return eax_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_ebx(u32 ebx) -> Self & {
            ebx_ = ebx;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto ebx() -> u32 {
            return ebx_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_ecx(u32 ecx) -> Self & {
            ecx_ = ecx;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto ecx() -> u32 {
            return ecx_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_edx(u32 edx) -> Self & {
            edx_ = edx;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto edx() -> u32 {
            return edx_;
        }
    private:
        u32 eax_;
        u32 ebx_;
        u32 ecx_;
        u32 edx_;
    };
} // namespace arch

#endif // #ifndef ARCH_X86_CPUID_HPP
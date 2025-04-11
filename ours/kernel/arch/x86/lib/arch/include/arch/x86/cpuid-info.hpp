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
#ifndef ARCH_X86_CPUID_INFO_HPP
#define ARCH_X86_CPUID_INFO_HPP 1

#include <arch/x86/cpuid-observer.hpp>

namespace arch {
    struct CpuIdInfo: public CpuIdObservable<CpuIdInfo> {
        auto init() -> void;
        
        template <typename Fn>
        auto for_each_cpuid(Fn &&fn) -> void {
            CpuId request;
            for (auto base = 0; base < max_cpuid; ++base) {
                request.query(CpuIdLeaf(base));
                fn(request.eax(), request.ebx(), request.ecx(), request.edx());
            }
        }

        template <typename Fn>
        auto for_each_ext_cpuid(Fn &&fn) -> void {
            CpuId request;
            for (auto base = 0x40000000; base < max_hyp_cpuid; ++base) {
                request.query(CpuIdLeaf(base));
                fn(request.eax(), request.ebx(), request.ecx(), request.edx());
            }
        }

        template <typename Fn>
        auto for_each_hyper_cpuid(Fn &&fn) -> void {
            CpuId request;
            for (auto base = 0x80000000; base < max_ext_cpuid; ++base) {
                request.query(CpuIdLeaf(base));
                fn(request.eax(), request.ebx(), request.ecx(), request.edx());
            }
        }

        u32 max_cpuid;
        u32 max_ext_cpuid;
        u32 max_hyp_cpuid;
    };

    struct CpuIdInfoObserver;
    using CpuIdInfoObserverBase = CpuIdObserver<CpuIdInfoObserver,
                                    CpuIdObserveItem<CpuIdLeaf::Vendor, CpuIdSubLeaf(0), CpuIdRegTags::Eax>,
                                    CpuIdObserveItem<CpuIdLeaf::HypVendor, CpuIdSubLeaf(0), CpuIdRegTags::Eax>,
                                    CpuIdObserveItem<CpuIdLeaf::IntelExtended, CpuIdSubLeaf(0), CpuIdRegTags::Eax>
                                  >;

    struct CpuIdInfoObserver: public CpuIdInfoObserverBase {
        typedef CpuIdInfo   InfoType;

        template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags Reg>
        auto observe(u32 value) -> bool;

        template <>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe<CpuIdLeaf::Vendor, CpuIdSubLeaf(0), CpuIdRegTags::Eax>(u32 value) -> bool {
            max_cpuid = value;
            return true;
        }

        template <>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe<CpuIdLeaf::HypVendor, CpuIdSubLeaf(0), CpuIdRegTags::Eax>(u32 value) -> bool {
            max_hyp_cpuid = value;
            return true;
        }

        template <>
        FORCE_INLINE CXX11_CONSTEXPR
        auto observe<CpuIdLeaf::IntelExtended, CpuIdSubLeaf(0), CpuIdRegTags::Eax>(u32 value) -> bool {
            max_ext_cpuid = value;
            return true;
        }

        auto finish() -> bool {
            cpuidinfo->max_cpuid = max_cpuid;
            cpuidinfo->max_ext_cpuid = max_ext_cpuid;
            cpuidinfo->max_hyp_cpuid = max_hyp_cpuid;
            return true;
        }

        CpuIdInfo *cpuidinfo;
        u32 max_cpuid;
        u32 max_ext_cpuid;
        u32 max_hyp_cpuid;
    };


} // namespace arch

#endif // #ifndef ARCH_X86_CPUID_INFO_HPP

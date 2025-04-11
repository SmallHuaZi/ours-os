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
#ifndef ARCH_PROCESSOR_HPP
#define ARCH_PROCESSOR_HPP 1

#include <arch/processor/vendor.hpp>
#include <arch/processor/topology.hpp>
#include <arch/processor/feature.hpp>
#include <arch/processor/cache.hpp>
#include <arch/processor/version.hpp>

namespace arch {
    struct X86CpuInfo {
        auto init() -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        auto feature() -> X86CpuFeature & {
            return features_;
        }

        Vendor vendor_;
        CpuTopologyNode topology_;
        X86CpuCacheInfo cache_;
        X86CpuFeature features_;
        X86CpuVersionInfo version_;
    };

    typedef X86CpuInfo      ArchCpuInfo;
} // namespace arch

#endif // #ifndef ARCH_PROCESSOR_HPP
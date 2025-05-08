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
#ifndef OURS_ARCH_X86_FEATURES_HPP
#define OURS_ARCH_X86_FEATURES_HPP 1

#include <arch/processor/feature.hpp>

namespace ours {
    using arch::CpuFeatureType;

    auto x86_has_feature(CpuFeatureType type) -> bool;

    extern bool g_feature_has_fsgsbase;

} // namespace ours

#endif // #ifndef OURS_ARCH_X86_FEATURES_HPP
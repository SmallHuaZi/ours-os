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

#ifndef OURS_MEM_GAF_POLICY_HPP
#define OURS_MEM_GAF_POLICY_HPP 1

#include <ours/mem/node_mask.hpp>

namespace ours::mem {
    enum class GafPolicyType {
        PreferNodeAffinity,
        PreferFramePriority,

        Default,
    };

    struct GafPolicy
    {
        GafPolicyType policy;
        NodeMask expected_nodes;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_GAF_POLICY_HPP
// arch/cpu ARCH/CPU_TOPOLOGY_HPP
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
#ifndef ARCH_CPU_TOPOLOGY_HPP
#define ARCH_CPU_TOPOLOGY_HPP 1

#include <arch/types.hpp>

namespace arch {
    struct CpuTopologyNode {
        // Real APIC ID read from the local APIC
        u32	apicid;

    	// The initial APIC ID provided by CPUID
    	u32	initial_apicid;

    	// Physical package ID
    	u32	pkg_id;

    	// Physical die ID on AMD, Relative on Intel
    	u32	die_id;

    	// Compute unit ID - AMD specific
    	u32	cu_id;

    	// Core ID relative to the package
    	u32	core_id;

    	// Logical ID mappings
    	u32	logical_pkg_id;
    	u32	logical_die_id;

    	// AMD Node ID and Nodes per Package info
    	u32	amd_node_id;

    	// Cache level topology IDs
    	u32	llc_id;
    	u32	l2c_id;
    };

} // namespace arch

#endif // #ifndef ARCH_CPU_TOPOLOGY_HPP
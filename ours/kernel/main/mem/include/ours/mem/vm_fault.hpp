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

#ifndef OURS_MEM_VM_FAULT_HPP
#define OURS_MEM_VM_FAULT_HPP 1

#include <ours/macro_abi.hpp>
#include <ustl/util/enum_bits.hpp>

namespace ours::mem {
    enum class VmfCause {
        None,
        Write = BIT(0),
        NotPresent = BIT(0),
        User = BIT(0),
    };
    USTL_ENABLE_ENUM_BITMASK(VmfCause);

    struct VmFault 
    {};

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_FAULT_HPP
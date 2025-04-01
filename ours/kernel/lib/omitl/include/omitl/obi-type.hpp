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
#ifndef OMITL_OBI_TYPE_HPP
#define OMITL_OBI_TYPE_HPP 1

#include <omitl/macro.hpp>
#include <ours/types.hpp>

namespace omitl {
    enum class ObiType: u32 {
        Container     =  OMIT_CONTAINER,
        KernelPackage =  OMIT_KPACKAGE,
        Ramdisk       =  OMIT_RAMDISK,
        Memory        =  OMIT_MEMORY,
    };

} // namespace omitl

#endif // #ifndef OMITL_OBI_TYPE_HPP 
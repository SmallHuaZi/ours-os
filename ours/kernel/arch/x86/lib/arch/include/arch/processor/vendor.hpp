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
#ifndef ARCH_PROCESSOR_VENDOR_HPP
#define ARCH_PROCESSOR_VENDOR_HPP 1

#include <arch/types.hpp>
#include <ustl/array.hpp>

namespace arch {
    enum class Vendor { 
        Intel,
        Amd,
        Unknown,
    };

    static char const * kVendorString[] {
        "GenuineIntel",
        "AuthenticAMD"
    };

} // namespace arch

#endif // #ifndef ARCH_PROCESSOR_VENDOR_HPP
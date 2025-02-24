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

#ifndef OURS_MEM_VM_PAGE_PROVIDER_HPP
#define OURS_MEM_VM_PAGE_PROVIDER_HPP 1

#include <ours/status.hpp>

namespace ours::mem {
    class VmPageProvider
    {
    public:
        virtual auto supply_pages() -> Status;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_PAGE_PROVIDER_HPP
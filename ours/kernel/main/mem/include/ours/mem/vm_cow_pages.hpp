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

#ifndef OURS_MEM_VM_COW_PAGES_HPP
#define OURS_MEM_VM_COW_PAGES_HPP 1

namespace ours::mem {
    /// A group of the copy on write page .
    class VmCowPages
    {
    public:
        class LookupCursor;
    };

    class VmCowPages::LookupCursor
    {
    public:
        auto require_owned_page() -> void;
        auto require_readonly_page() -> void;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_VM_COW_PAGES_HPP
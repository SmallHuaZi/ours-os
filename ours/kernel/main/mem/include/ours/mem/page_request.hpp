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
#ifndef OURS_MEM_PAGE_REQUEST_HPP
#define OURS_MEM_PAGE_REQUEST_HPP 1

#include <ours/types.hpp>
#include <ours/status.hpp>

#include <ustl/rc.hpp>
#include <ktl/xarray.hpp>

namespace ours::mem {
    class PageRequest {
    public:
        auto wait() -> Status;

    private:
        PgOff vmo_off;
        usize num_pages;
    };

    class PageSource: public ustl::RefCounter<PageSource> {
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_PAGE_REQUEST_HPP
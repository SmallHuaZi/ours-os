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
#ifndef ARCH_PAGING_VISITORS_HPP
#define ARCH_PAGING_VISITORS_HPP 1

namespace arch::paging {
    template <typename PageSource>
    struct MapVisitor {};

    struct AltMapVisitor {};

    template <typename PageSource>
    struct UnMapVisitor {};

} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_VISITORS_HPP
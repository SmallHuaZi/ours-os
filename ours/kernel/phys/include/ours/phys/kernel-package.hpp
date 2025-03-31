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
#ifndef OURS_PHYS_KERNEL_PACKAGE_HPP
#define OURS_PHYS_KERNEL_PACKAGE_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/status.hpp>

#include <arch/tick.hpp>
#include <omitl/obi-view.hpp>

namespace ours::phys {
    /// It ownes a group of OBI which contain `main.image`, `initrd` and e.g.
    struct KernelPackage {
        typedef omitl::ObiView  Obi;
        typedef Obi::Iterator   Item;

        auto init(Obi const &obi) -> Status;

        FORCE_INLINE CXX11_CONSTEXPR
        auto init(PhysAddr obi) -> Status {
            return init(Obi::from(reinterpret_cast<omitl::ObiHeader *>(obi)));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto crate() const -> Item {
            return package_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto package() const -> ustl::views::Span<u8> {
            return package_loaded_;
        }
    private:
        auto unpack() -> Status;

        Obi obi_;
        Item package_;
        ustl::views::Span<u8> package_loaded_;
        arch::Tick package_load_start_;
        arch::Tick package_load_end_;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_KERNEL_PACKAGE_HPP
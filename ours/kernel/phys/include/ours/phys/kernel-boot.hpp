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
#ifndef OURS_PHYS_KERNEL_BOOT_HPP
#define OURS_PHYS_KERNEL_BOOT_HPP 1

#include <ours/types.hpp>

namespace ours::phys {
    struct KernelPackage;

    struct KernelBoot {
        auto boot(KernelPackage &kpackage) -> void;
    private:
        auto build_kernel_aspace() -> void;
        auto do_handoff() -> void;
        auto arch_do_handoff() -> void;

        template <typename... Args>
        auto start_kernel(Args... args) -> void {
            (*reinterpret_cast<auto (*)(Args...) -> void>(kernel_entry_))(args...);
        }
        
        VirtAddr kernel_size_;
        VirtAddr kernel_entry_;
        PhysAddr kernel_addr_;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_KERNEL_BOOT_HPP
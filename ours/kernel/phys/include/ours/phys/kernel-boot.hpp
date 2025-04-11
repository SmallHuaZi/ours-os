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

#include <ours/phys/boot-options.hpp>
#include <ours/phys/init-fs.hpp>
#include <ours/phys/kernel-image.hpp>
#include <ours/arch/aspace_layout.hpp>

#include <ustl/option.hpp>
#include <ustl/views/span.hpp>
#include <ustl/function/fn.hpp>
#include <ustl/collections/intrusive/slist.hpp>

namespace ours::phys {
    struct Aspace;
    struct KernelPackage;

    struct KernelBoot {
        auto init(KernelPackage &kpackage) -> void;

        auto load() -> void;

        auto boot() -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        auto image() -> KernelImage & {
            return kimage_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto load_address() -> VirtAddr {
            return KERNEL_LOAD_BASE + kaslr_offset_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto kernel_addr() const -> PhysAddr {
            return kernel_addr_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto kernel_size() const -> usize {
            return kernel_size_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto kernel_entry_point() const -> usize {
            return kernel_entry_;
        }
    private:
        auto gen_load_bias(usize const limit) -> usize;
        auto build_kernel_aspace() -> void;
        auto do_handoff() -> void;
        auto arch_do_handoff() -> void;

        template <typename... Args>
        auto invoke_kernel(Args... args) -> void {
            (*reinterpret_cast<auto (*)(Args...) -> void>(kernel_entry_))(args...);
        }

        InitFs   initfs_;
        PhysAddr kernel_addr_;
        PhysAddr kernel_size_;
        VirtAddr kernel_entry_;
        usize kaslr_offset_;
        KernelImage kimage_;
    };

} // namespace ours::phys

#endif // #ifndef OURS_PHYS_KERNEL_BOOT_HPP
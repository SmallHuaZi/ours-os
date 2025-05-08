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
#ifndef ARCH_X86_FSGS_HPP
#define ARCH_X86_FSGS_HPP 1

#include <arch/types.hpp>

namespace arch {
    template <typename Integer>
    FORCE_INLINE
    static auto read_gs_offset(usize offset) -> Integer {
        Integer ret;
        asm volatile("mov %%gs:%1, %0" : "=r"(ret) : "m"(*(Integer *)offset) : "memory");
        return ret;
    }

    template <typename Integer>
    FORCE_INLINE
    static auto write_gs_offset(usize offset, Integer val) -> void {
        asm volatile("mov %0, %%gs:%1" :: "ir"(val), "m"(*(Integer *)offset) : "memory");
    }

    template <typename Integer>
    FORCE_INLINE
    static auto read_fsbase() -> Integer {
        Integer out;
        asm volatile("rdfsbase %0" :"=r"(out) ::"memory");
        return out;
    } 

    template <typename Integer>
    FORCE_INLINE
    static auto write_fsbase(Integer val) -> void {
        asm volatile("wrfsbase %0" ::"r"(val) :"memory");
    } 

    template <typename Integer>
    FORCE_INLINE
    static auto read_gsbase() -> Integer {
        Integer out;
        asm volatile("rdgsbase %0" :"=r"(out) ::"memory");
        return out;
    } 

    template <typename Integer>
    FORCE_INLINE
    static auto write_gsbase(Integer val) -> void {
        asm volatile("wrgsbase %0" ::"r"(val) :"memory");
    } 

    FORCE_INLINE
    static auto swap_gs() -> void {
        asm volatile("swapgs" :::"memory");
    } 

    template <typename Integer>
    FORCE_INLINE
    static auto exchange_gsbase_after_swap(Integer val) -> Integer {
        Integer out;
        asm volatile("swapgs;" 
                     "rdgsbase %0;" 
                     "wrgsbase %1;" 
                     "swapgs;"
                     :"=r"(out) :"r"(val) :"memory"
        );
    } 

} // namespace arch

#endif // #ifndef ARCH_X86_FSGS_HPP
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

#ifndef OURS_CPU_MASK_HPP
#define OURS_CPU_MASK_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/cpu_cfg.hpp>

#include <ustl/bitset.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/traits/is_invocable.hpp>

namespace ours {
    struct CpuMask
        : public ustl::BitSet<MAX_CPU_NUM>
    {
        typedef CpuMask                     Self;
        typedef ustl::BitSet<MAX_CPU_NUM>   Base;

        static Self  POSSIBLE_CPUS;
        static Self  ONLINE_CPUS;
        static Self  ACTIVE_CPUS;
    };

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    auto for_each_cpu(CpuMask const &cpu_mask, F &&functor) -> void
    {
        for (auto i = 0; i < MAX_CPU_NUM; ++i) {
            if (cpu_mask.test(i)) {
                ustl::function::invoke(functor, CpuNum(i));
            }
        }
    }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE
    auto for_each_possible_cpu(F &&functor) -> void
    {  return for_each_cpu(CpuMask::POSSIBLE_CPUS, functor);  }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE 
    auto for_each_online_cpu(F &&functor) -> void
    {  return for_each_cpu(CpuMask::ONLINE_CPUS, functor);  }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE 
    auto for_each_active_cpu(F &&functor) -> void
    {  return for_each_cpu(CpuMask::ACTIVE_CPUS, functor);  }

} // namespace ours

#endif // #ifndef OURS_CPU_MASK_HPP
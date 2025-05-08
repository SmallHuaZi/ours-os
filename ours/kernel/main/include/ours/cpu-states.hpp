// ours OURS_CPU-STATES_HPP
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
#ifndef OURS_CPU_STATES_HPP
#define OURS_CPU_STATES_HPP 1

#include <ours/cpu-mask.hpp>
#include <ours/arch/cpu.hpp>

namespace ours {
    struct CpuStates {
        typedef CpuStates   Self;

        FORCE_INLINE
        auto set_possible(CpuNum cpunum, bool possible) -> void {
            DEBUG_ASSERT(cpunum < MAX_CPU, "");
            possible_cpus.set(cpunum, possible);
        }

        FORCE_INLINE
        auto set_online(CpuNum cpunum, bool online) -> void {
            DEBUG_ASSERT(cpunum < MAX_CPU, "");
            online_cpus.set(cpunum, online);
        }

        FORCE_INLINE
        auto set_active(CpuNum cpunum, bool active) -> void {
            DEBUG_ASSERT(cpunum < MAX_CPU, "");
            possible_cpus.set(cpunum, active);
        }

        CpuMask  possible_cpus;
        CpuMask  online_cpus;
        CpuMask  active_cpus;
        ustl::sync::AtomicU32 nr_onlines;

        static CpuStates s_global_cpu_states;
    };
    inline CpuStates CpuStates::s_global_cpu_states;

    FORCE_INLINE
    static auto global_cpu_states() -> CpuStates & {
        return CpuStates::s_global_cpu_states;
    }

    FORCE_INLINE
    static auto cpu_possible_mask() -> CpuMask & {
        return global_cpu_states().possible_cpus;
    }

    FORCE_INLINE
    static auto cpu_online_mask() -> CpuMask & {
        return global_cpu_states().online_cpus;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto num_possible_cpus() -> usize {
        return cpu_possible_mask().count();
    }


    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE
    auto for_each_possible_cpu(F &&functor) -> void {  
        return for_each_cpu(global_cpu_states().possible_cpus, functor);  
    }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE 
    auto for_each_online_cpu(F &&functor) -> void {  
        return for_each_cpu(global_cpu_states().online_cpus, functor);  
    }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    FORCE_INLINE 
    auto for_each_active_cpu(F &&functor) -> void {  
        return for_each_cpu(global_cpu_states().active_cpus, functor);  
    }

    FORCE_INLINE
    static auto set_current_cpu_online(bool online) -> void {
        global_cpu_states().set_online(arch_current_cpu(), online);
    }
} // namespace ours

#endif // #ifndef OURS_CPU_STATES_HPP
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
#include <ours/cpu-cfg.hpp>
#include <ours/status.hpp>
#include <ours/assert.hpp>

#include <ustl/bit.hpp>
#include <ustl/bitset.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/function/invoke.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/traits/invoke_result.hpp>
#include <ustl/traits/is_same.hpp>

namespace ours {
    struct CpuMask: public ustl::BitSet<MAX_CPU> {
        typedef CpuMask                 Self;
        typedef ustl::BitSet<MAX_CPU>   Base;
        using Base::Base;
        using Base::operator=;
        using Base::operator|=;
        using Base::operator&=;
        using Base::operator^=;
        using Base::operator<<=;
        using Base::operator>>=;

        CpuMask() = default;

        CpuMask(Base const &base)
            : Base(base)
        {}

        static auto from_cpu_num(CpuNum cpunum) -> Self {
            Self self;
            self.set(cpunum);
            return self;
        }
    };

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum>
    auto for_each_cpu(CpuMask const &cpu_mask, F &&functor) -> void {
        for (auto i = 0; i < cpu_mask.size(); ++i) {
            if (cpu_mask.test(i)) {
                ustl::function::invoke(functor, CpuNum(i));
            }
        }
    }

    template <typename F>
        requires ustl::traits::Invocable<F, CpuNum> &&
                 ustl::traits::IsSameV<ustl::traits::InvokeResultT<F, CpuNum>, Status>
    auto for_each_cpu(CpuMask const &cpu_mask, F &&functor) -> void {
        for (auto i = 0; i < cpu_mask.size(); ++i) {
            if (!cpu_mask.test(i)) {
                continue;
            }
            auto const status = ustl::function::invoke(functor, CpuNum(i));
            if (Status::Ok == status) {
                return;
            }
        }
    }

} // namespace ours

#endif // #ifndef OURS_CPU_MASK_HPP
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
#ifndef ARCH_X86_CPUID_OBSERVER_HPP
#define ARCH_X86_CPUID_OBSERVER_HPP 1

#include <arch/x86/cpuid.hpp>

#include <ustl/util/tuple.hpp>

namespace arch {
    enum class CpuIdRegTags {
        Eax,
        Ebx,
        Ecx,
        Edx,
    };

    template <typename Derived>
    struct CpuIdObservable {};

    /// TODO(SmallHUaZi) The static observer design mode is our final form for decoding the output from CPUID function.
    /// It can be help us avoid very much of meaningless duplication on the code. For instance, the current `ArchCpuInfo`
    /// class has a `ArchCpuFeature` member, and those all perform the zero leaf function to read and save manufacturer of
    /// processor. Through the design mode, we can seprate them to the independent part, in the meanwhile the invoking times
    /// of the same CPUID functions would descend explicitly.
    /// 
    template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf>
    struct CpuIdAction {
        CXX11_CONSTEXPR
        static auto const kLeaf = Leaf;

        CXX11_CONSTEXPR
        static auto const kSubLeaf = SubLeaf;
    };

    template <typename CpuIdAction, CpuIdRegTags... Regs>
    struct CpuIdObserveItemInner;

    template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags... Regs>
    struct CpuIdObserveItemInner<CpuIdAction<Leaf, SubLeaf>, Regs...> {
        typedef CpuIdAction<Leaf, SubLeaf>  Action;
    };

    template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags... Regs>
    using CpuIdObserveItem = CpuIdObserveItemInner<CpuIdAction<Leaf, SubLeaf>, Regs...>;

    template <typename Derived, typename... ObservedItems>
    struct CpuIdObserver {
        typedef CpuIdObserver<Derived, ObservedItems...>    Base;
        typedef ustl::TypeList<ObservedItems...>            Items;

        /// For debug
        template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags Reg, typename Integral>
        auto observe(Integral value) -> bool {
            DEBUG_ASSERT(false);
            return true;
        }
    };

    template <typename... OtherObservers>
    struct ExtractActionsFromObservers {
        typedef ustl::TypeList<>    Type;
    };

    template <typename Observer, typename... OtherObservers>
    struct ExtractActionsFromObservers<Observer, OtherObservers...> {
        template <typename Item>
        struct ItemToAction {
            typedef typename Item::Action   Type;
        };

        typedef typename ExtractActionsFromObservers<OtherObservers...>::Type           NextIterationResult;
        typedef ustl::TypeAlgos::Transform<typename Observer::Items, ItemToAction>      ThisActions;
        typedef ustl::TypeAlgos::Merge<ThisActions, NextIterationResult>                ActionsMayDuplicated;
        typedef ustl::TypeAlgos::Unique<ActionsMayDuplicated>    Type;
    };

    template <typename Action, typename CpuId, typename Observer, CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags... Regs>
    CXX11_CONSTEXPR
    auto dispatch_cpuid_3(CpuId &result, Observer &&observer,
                          CpuIdObserveItem<Leaf, SubLeaf, Regs...>) -> bool {
        if CXX17_CONSTEXPR (Action::kLeaf != Leaf || Action::kSubLeaf != SubLeaf) {
            return true;
        }

        return (observer.template observe<Leaf, SubLeaf, Regs>(
                                       Regs == CpuIdRegTags::Eax ? result.eax() :
                                       Regs == CpuIdRegTags::Ebx ? result.ebx() :
                                       Regs == CpuIdRegTags::Ecx ? result.ecx() :
                                       result.edx()) && ...);
    }

    template <typename Action, typename CpuId, typename Observer, typename... ObserveItems>
    CXX11_CONSTEXPR
    auto dispatch_cpuid_2(CpuId &result, CpuIdObserver<Observer, ObserveItems...> &&observer) -> bool {
        return (dispatch_cpuid_3<Action>(result, static_cast<Observer &>(observer), ObserveItems()) && ...);
    }

    template <typename Action, typename CpuId, typename... Observers>
    CXX11_CONSTEXPR
    auto dispatch_cpuid_1(CpuId &result, Observers &&...observers) -> bool {
        return (dispatch_cpuid_2<Action>(result, ustl::forward<Observers>(observers)), ...);
    }

    /// Assume that there are not duplicated items in the `Actions`
    template <typename CpuId, typename... Actions, typename... Observers>
    CXX11_CONSTEXPR
    auto dispatch_cpuid_0(CpuId &&cpuid, ustl::TypeList<Actions...>, Observers &&...observers) -> bool {
        return ((cpuid.query_safe(Actions::kLeaf, Actions::kSubLeaf) &&
                 dispatch_cpuid_1<Actions>(cpuid, ustl::forward<Observers>(observers)...)) && ...);
    }

    /// Do the following thins as the step:
    ///  1). Extract the required CPUID leaf and subleaf information from the provided Observers...;
    ///  2). Execute the corresponding CPUID instructions;
    ///  3). Distribute the results to each observer for decoding and interpretation;
    ///  4). Return a boolean indicating whether all operations completed successfully.
    template <typename... Observers>
    auto dispatch_cpuid(Observers &&...observers) -> bool {
        typedef typename ExtractActionsFromObservers<Observers...>::Type   Actions;
        CpuId cpuid;
        return dispatch_cpuid_0(cpuid, Actions(), ustl::forward<Observers>(observers)...);
    }

    struct AddrSizeCpuIdObserver
        : CpuIdObserver<AddrSizeCpuIdObserver, 
                        CpuIdObserveItem<CpuIdLeaf::IntelAddrWidth, CpuIdSubLeaf(0), CpuIdRegTags::Eax>> {
        AddrSizeCpuIdObserver(u8 &max_phys_bits, u8 &max_virt_bits) 
            : max_phys_bits(&max_phys_bits), max_virt_bits(&max_virt_bits)
        {}

        template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags Reg, typename Integral>
        auto observe(Integral value) -> bool {
            u8 const phys_bits = value & 0xFF;
            u8 const virt_bits = (value >> 8) & 0xFF;

            // The value may be zero.
            if (phys_bits > *max_phys_bits) {
                *max_phys_bits = phys_bits;
            }

            if (virt_bits > *max_virt_bits) {
                *max_virt_bits = virt_bits;
            }
            return true;
        }

        u8 *max_phys_bits;
        u8 *max_virt_bits;
    };

} // namespace arch

#endif // #ifndef ARCH_X86_CPUID_OBSERVER_HPP
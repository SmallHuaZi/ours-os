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
#ifndef EFTL_ELF_OBSERVABLE_HPP
#define EFTL_ELF_OBSERVABLE_HPP 1

#include <ours/config.hpp>
#include <eftl/elf/traits.hpp>

#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_base_of.hpp>

namespace eftl::elf {
    struct EflTraits32;
    struct EflTraits64;

    /// Base helper class implementing CRTP to implicitly propagate `ElfTraits` template parameters.
    ///
    /// This base class enables derived classes to reuse type traits without explicitly specifying
    /// `ElfTraits` template parameters through static polymorphism via the Curiously Recurring Template Pattern.
    ///
    /// #### Typical Usage
    /// Derived classes should inherit via CRTP:
    /// ```cpp
    /// class Segment32
    ///     : public Object<ElfClass32, Derived>
    /// { ... };
    /// class Segment64
    ///     : public Object<ElfClass64, Derived>
    /// { ... };
    /// 
    /// typedef ElfView<ElfTraits32> ElfView;
    /// ElfView view;
    /// ElfView::Segment segment;
    /// decode_segments(view.binary(), view.segments(), Segment.make_observer<LoadSegObserver>());
    ///
    /// // If no this, we must explicitly specify the class of ELF traits, like the following showed,
    /// decode_segments(view.binary(), view.segments(), LoadSegObserver<ElfTraits32>(segment));
    /// // or
    /// decode_segments(view.binary(), view.segments(), LoadSegObserver<ElfView::ElfTraits>(segment));
    /// ```
    ///
    /// #### Design Motivation
    /// - **Type Erasure Avoidance**: Eliminates runtime overhead from type erasure
    /// - **Template Parameter Simplification**: Reduces boilerplate of passing `ElfTraits` explicitly
    /// - **Static Polymorphism**: Enables efficient compile-time method dispatch
    ///
    /// @tparam Derived CRTP-derived class type
    template <typename ElfTraits, typename Derived>
    struct Observable {
        static_assert(ustl::traits::IsSameV<ElfTraits, ElfTraits32> || 
                      ustl::traits::IsSameV<ElfTraits, ElfTraits64>,
                      "Unknown elf traits type");

        template <template <typename Traits> typename Observer>
        FORCE_INLINE CXX11_CONSTEXPR
        auto make_observer() -> Observer<ElfTraits> {
            static_assert(ustl::traits::IsBaseOfV<Observable, Derived>, 
                          "`Derived` must inherit from `Observable`");

            return { *static_cast<Derived *>(this) };
        }
    };

} // namespace eftl::elf

#endif // EFTL_ELF_OBSERVABLE_HPP

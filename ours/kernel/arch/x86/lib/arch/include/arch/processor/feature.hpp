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
#ifndef ARCH_CPU_FEATURE_HPP
#define ARCH_CPU_FEATURE_HPP 1

#include <arch/types.hpp>
#include <arch/processor/vendor.hpp>
#include <arch/x86/cpuid-observer.hpp>

#include <ustl/bitset.hpp>
#include <ustl/util/type-map.hpp>
#include <ustl/util/enum_bits.hpp>
#include <ustl/traits/integral_constant.hpp>

namespace arch {
    enum class CpuFeatureType {
#define FEATURE(NAME, VALUE)    NAME = (VALUE),
#   include <arch/processor/feature-intel.def>
#undef FEATURE
        MaxNumFeatures,
    };
    USTL_ENABLE_ENUM_BITMASK(CpuFeatureType);

    struct X86CpuFeature: public CpuIdObservable<X86CpuFeature> {
        typedef X86CpuFeature   Self;

        typedef ustl::TypeList<
            // CPUID 0x00000000:0
            CpuIdObserveItem<CpuIdLeaf::Features, CpuIdSubLeaf(0), CpuIdRegTags::Edx>,
            CpuIdObserveItem<CpuIdLeaf::Features, CpuIdSubLeaf(0), CpuIdRegTags::Ecx>,
            // CPUID 0x00000007:0. The extended features
            CpuIdObserveItem<CpuIdLeaf::ExtendedFeature, CpuIdSubLeaf(0), CpuIdRegTags::Ebx>,
            // CPUID 0x00000007:0. The extended features
            CpuIdObserveItem<CpuIdLeaf::ExtendedFeature, CpuIdSubLeaf(0), CpuIdRegTags::Ecx>,
            // CPUID 0x00000007:0. The extended features
            CpuIdObserveItem<CpuIdLeaf::ExtendedFeature, CpuIdSubLeaf(0), CpuIdRegTags::Edx>,
            // CPUID 0x00000007:1. The extended features
            CpuIdObserveItem<CpuIdLeaf::ExtendedFeature, CpuIdSubLeaf(1), CpuIdRegTags::Eax>,
            // CPUID 0x00000007:1. The extended features
            CpuIdObserveItem<CpuIdLeaf::Amd80000007EBX, CpuIdSubLeaf(0), CpuIdRegTags::Ebx>
        > ItemList;

        template <typename ObservedItem>
        CXX11_CONSTEXPR
        static auto const IndexV = ustl::TypeAlgos::GetPosV<ItemList, ObservedItem>;

        struct SelfCpuIdObserver;

        template <typename ItemList>
        struct MakeSelfCpuIdObserverBase;

        template <typename... Items>
        struct MakeSelfCpuIdObserverBase<ustl::TypeList<Items...>> {
            typedef arch::CpuIdObserver<SelfCpuIdObserver, Items...>     Type;
        };

        struct SelfCpuIdObserver: MakeSelfCpuIdObserverBase<ItemList>::Type {
            SelfCpuIdObserver(X86CpuFeature &feature)
                : feature_(&feature)
            {}

            template <CpuIdLeaf Leaf, CpuIdSubLeaf SubLeaf, CpuIdRegTags Reg, typename Integral>
            FORCE_INLINE CXX11_CONSTEXPR
            auto observe(Integral value) -> bool {
                static_assert(X86CpuFeature::IndexV<CpuIdObserveItem<Leaf, SubLeaf, Reg>> != ustl::TypeAlgos::kNotFound);

                CXX11_CONSTEXPR 
                static auto const kDelta = X86CpuFeature::IndexV<CpuIdObserveItem<Leaf, SubLeaf, Reg>> *
                                           ustl::NumericLimits<Integral>::DIGITS;
                static_assert(kDelta <= kMaxNumFeatures);

                ustl::copy_bits<kDelta>(feature_->features_, value);
                return true;
            }

            X86CpuFeature *feature_;
        };

        CXX11_CONSTEXPR
        static auto const kMaxNumFeatures = usize(CpuFeatureType::MaxNumFeatures);

        FORCE_INLINE CXX11_CONSTEXPR
        X86CpuFeature()
            : features_()
        {}

        FORCE_INLINE CXX11_CONSTEXPR
        auto has_feature(CpuFeatureType feature) -> bool {
            DEBUG_ASSERT(usize(feature) < kMaxNumFeatures);
            return features_.test(usize(feature));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto enable_feature(CpuFeatureType feature) -> Self & {
            DEBUG_ASSERT(usize(feature) < kMaxNumFeatures);
            features_.set(usize(feature));
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto disable_feature(CpuFeatureType feature) -> Self & {
            DEBUG_ASSERT(usize(feature) < kMaxNumFeatures);
            features_.set(usize(feature), 0);
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto make_cpuid_observer() -> SelfCpuIdObserver {
            return SelfCpuIdObserver(*this);
        }
    private:
        ustl::BitSet<kMaxNumFeatures> features_;
    };

    typedef X86CpuFeature   ArchCpuFeature; 

} // namespace arch

#endif // #ifndef ARCH_CPU_FEATURE_HPP
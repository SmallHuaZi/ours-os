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

#ifndef USTL_BITFIELDS_HPP
#define USTL_BITFIELDS_HPP 1

#include <ustl/bit.hpp>
#include <ustl/config.hpp>
#include <ustl/limits.hpp>
#include <ustl/initializer_list.hpp>
#include <ustl/util/fold.hpp>
#include <ustl/util/find_if.hpp>
#include <ustl/util/types_list.hpp>
#include <ustl/util/fixed_string.hpp>
#include <ustl/util/index_sequence.hpp>
#include <ustl/util/pack_options.hpp>
#include <ustl/util/minmax.hpp>
#include <ustl/traits/integral.hpp>
#include <ustl/traits/is_convertible.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_trivially_constructible.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/algorithms/copy.hpp>

/// Since C++17
///    1) CTAD.
///    2) Auto template arguments. 

namespace ustl {
namespace bitfields {
    template<int Id, int Bits, typename Type, util::FixedString Name, bool Enable>
    struct FieldMetaData
    {
        typedef Type    ValueType;

        USTL_CONSTEXPR
        static auto const ID = Id;

        USTL_CONSTEXPR
        static auto const START_BIT = Bits;

        USTL_CONSTEXPR
        static auto const BITS = Bits;

        USTL_CONSTEXPR
        static auto const NAME = Name;

        USTL_CONSTEXPR
        static auto const ENABLE = Enable;
    };

    template<typename... Fields>
    using FieldList = TypeList<Fields...>;

    /// Not be used now
    enum class IdPolicy {
        Auto,
        Manually,
    };

    /// HasDuplicates
    template <typename FieldList>
    struct HasDuplicates;

    template <>
    struct HasDuplicates<TypeList<>>
        : traits::FalseType
    {};

    template <typename HeadField>
    struct HasDuplicates<TypeList<HeadField>>
        : traits::FalseType
    {};

    template <typename HeadField, typename... RemainingFields>
    struct HasDuplicates<TypeList<HeadField, RemainingFields...>>
        : traits::IntegralConstantInterface<HasDuplicates<TypeList<HeadField, RemainingFields...>>, bool>
    {
        template <typename Field>
        struct IsDuplicatedWith
            : public traits::BoolConstant<HeadField::ID == Field::ID>  
        {  typedef bool    RetType;  };

        typedef HasDuplicates<TypeList<RemainingFields...>>   NextIteration;

        USTL_CONSTEXPR
        static bool const VALUE = {
            util::Fold<IsDuplicatedWith, util::Accumulator::BoolOr, RemainingFields...>::VALUE ||
            NextIteration::VALUE
        };
    };

    /// GetField<Id, FieldList>
    template <typename FieldList, usize Id>
    struct GetField;

    template <usize Id, typename... Fields>
    struct GetField<TypeList<Fields...>, Id>
    {
        template <typename Field>
        using Matcher = traits::BoolConstant<Id == Field::ID>;

        typedef typename util::FindIf<Matcher, Fields...>::Type  Type;
        static_assert(!traits::IsSameV<Type, Monostate>, "The type corresponding to `ID` not exists");
    };

    template <typename FieldList, usize Id>
    using GetFieldT = typename GetField<FieldList, Id>::Type;

    template <typename FieldList, usize Id>
    struct GetFieldBits
        : public traits::IntConstant<GetFieldT<FieldList, Id>::BITS>
    {};

    /// TotalOccupiedBits 
    template <typename FieldList>
    struct TotalOccupiedBits;

    template <typename... Fields>
    struct TotalOccupiedBits<TypeList<Fields...>>
        : traits::IntegralConstantInterface<TotalOccupiedBits<TypeList<Fields...>>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        template <typename Field>
        struct GetOccupiedBits
            : public traits::IntConstant<Field::BITS>
        {  typedef isize    RetType;  };

        USTL_CONSTEXPR
        static usize const VALUE = {
            util::Fold<GetOccupiedBits, util::Accumulator::IntAdd, Fields...>::VALUE
        };
    };

    template <typename FieldList>
    struct CalculateOccupiedUsize;

    template <typename... Fields>
    struct CalculateOccupiedUsize<TypeList<Fields...>>
        : traits::IntegralConstantInterface<CalculateOccupiedUsize<TypeList<Fields...>>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        USTL_CONSTEXPR
        static usize const BITS = ustl::NumericLimits<usize>::DIGITS;

        typedef ustl::traits::ConditionalT<sizeof...(Fields) == 0,
            traits::IntConstant<1>, 
            TotalOccupiedBits<FieldList>>   GetOccupiedBits;

        USTL_CONSTEXPR
        static usize const VALUE = (GetOccupiedBits() + BITS - 1) / BITS; // align up
    };

    template <typename FieldList, usize Start, usize End>
    struct TotalOccupiedUsizeRange;

    template <usize Start, usize End, typename... Fields>
    struct TotalOccupiedUsizeRange<TypeList<Fields...>, Start, End>
    {
        typedef TypeListSubList<TypeList<Fields...>, Start, End>::Type  TargetFields;

        USTL_CONSTEXPR
        static usize const VALUE = CalculateOccupiedUsize<TargetFields>();
    };

    template <typename FieldList, usize... Ids>
    struct TotalOccupiedUsizeByIds;

    template <typename... Fields, usize... Ids>
    struct TotalOccupiedUsizeByIds<TypeList<Fields...>, Ids...>
    {
        typedef TypeList<Fields...>     FieldList;

        template <typename Field>
        struct MatchWithGivenIds
        {
            template <typename Wrapper>
            struct Matcher
                : traits::BoolConstant<Field::ID == Wrapper()>
            {  typedef bool    RetType;  };

            USTL_CONSTEXPR
            static bool const VALUE = {
                util::Fold<Matcher, util::Accumulator::BoolOr, traits::IntConstant<Ids>...>::VALUE
            };
        };

        template <typename Field>
        struct GetOccupiedBits
        {  
            typedef usize    RetType;

            USTL_CONSTEXPR
            static auto const VALUE = MatchWithGivenIds<Field>::VALUE ? Field::BITS : 0;
        };

        USTL_CONSTEXPR
        static usize const VALUE = {
            util::Fold<GetOccupiedBits, util::Accumulator::IntAdd, Fields...>::VALUE
        };
    };

    /// FindBitPos
    template <typename FieldList, usize Id>
    struct FindBitPos;

    template <usize Id, typename... Fields>
    struct FindBitPos<TypeList<Fields...>, Id>
        : traits::IntegralConstantInterface<FindBitPos<TypeList<Fields...>, Id>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        template <typename Field>
        struct AddBitsIfIdLesserThanThis 
            : public traits::IntConstant<Field::ID < Id ? Field::BITS : 0>
        {  typedef isize    RetType;  };

        USTL_CONSTEXPR
        static usize const VALUE = {
            util::Fold<AddBitsIfIdLesserThanThis, util::Accumulator::IntAdd, Fields...>::VALUE
        };
    };

    /// FindUnderlyingTypeIndex
    template <typename FieldList, usize Id>
    struct FindUsizeIndex;

    template <usize Id, typename... Fields>
    struct FindUsizeIndex<TypeList<Fields...>, Id>
        : traits::IntegralConstantInterface<FindUsizeIndex<TypeList<Fields...>, Id>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        USTL_CONSTEXPR
        static usize const VALUE = FindBitPos<FieldList, Id>::VALUE / ustl::NumericLimits<usize>::DIGITS;
    };

    /// GetFieldMask
    template <typename FieldList, usize Id>
    struct GetFieldMask;

    template <usize Id, typename... Fields>
    struct GetFieldMask<TypeList<Fields...>, Id> 
        : traits::IntegralConstantInterface<GetFieldMask<TypeList<Fields...>, Id>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        USTL_CONSTEXPR
        static usize const BITPOS = FindBitPos<FieldList, Id>();

        USTL_CONSTEXPR
        static usize const BITS = GetFieldT<FieldList, Id>::BITS;

        USTL_CONSTEXPR
        static usize const VALUE = MakeBitMask<usize, BITPOS, BITS>::VALUE;
    };

    /// GetFieldShift
    template <typename FieldList, usize Id>
    struct GetFieldShift;

    template <usize Id, typename... Fields>
    struct GetFieldShift<TypeList<Fields...>, Id>
        : traits::IntegralConstantInterface<GetFieldShift<TypeList<Fields...>, Id>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        USTL_CONSTEXPR
        static usize const BITS = ustl::NumericLimits<usize>::DIGITS;

        USTL_CONSTEXPR
        static usize const VALUE = FindBitPos<FieldList, Id>() - FindUsizeIndex<FieldList, Id>() * BITS;
    };

    struct DefaultFieldOptions
    {
        typedef usize   Type;

        USTL_CONSTEXPR 
        static auto const BITS = 1;

        USTL_CONSTEXPR 
        static auto const ENABLE = true;

        USTL_CONSTEXPR 
        static util::FixedString const NAME{"Anonymous"};
    };

    template <typename... Options>
    struct MakeBitField
    {
        typedef typename PackOptions<DefaultFieldOptions, Options...>::Type 
            PackedOption;

        typedef FieldMetaData<
            PackedOption::ID, 
            PackedOption::ENABLE ?PackedOption::BITS : 0, 
            typename PackedOption::Type,
            PackedOption::NAME,
            PackedOption::ENABLE
        > Type;
    };

    template <typename... Options>
    struct BitField
        : public bitfields::MakeBitField<Options...>::Type
    {
        typedef typename bitfields::MakeBitField<Options...>::Type     Base;
        using Base::Base;
    };

    template<typename... Fields>
    class BitFields
        : public BitFields<FieldList<Fields...>>
    {
    public:
        typedef BitFields<FieldList<Fields...>>   Base;
        using Base::Base;
    };

    /// This option is used to support `operator|[=]` and `operator&[=]` for some enum fields.
    /// It is very optional, please ensure only one field specified the enum type, otherwise causing
    /// a static assertion error.
    USTL_CONSTANT_OPTION(EnableBitOpForEnumField, bool, ENABLE_BITOP_FOR_ENUM_FIELD);

    template<typename... Fields>
    class BitFields<FieldList<Fields...>>
    {
        typedef BitFields               Self;
        typedef TypeList<Fields...>     FieldList;
        static_assert(!bitfields::HasDuplicates<FieldList>::VALUE, "Has duplicated field");

        template <usize Id>
        using ValueTypeOf = typename bitfields::GetFieldT<FieldList, Id>::ValueType;

        template <usize Id>
        USTL_CONSTEXPR
        static auto const IsFieldEnabledV = bitfields::GetFieldT<FieldList, Id>::ENABLE;
    public:
        USTL_CONSTEXPR 
        BitFields() = default;
    
        USTL_CONSTEXPR 
        BitFields(ustl::InitializerList<usize> const &val) { 
            ustl::algorithms::copy_n(val.begin(), NUM_USIZE, values);  
        }
    
        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto set(ValueTypeOf<Id> const &value) USTL_NOEXCEPT -> void {
            priv_set<Id>(value, traits::BoolConstant<IsFieldEnabledV<Id>>()); 
        }
    
        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto get() const USTL_NOEXCEPT -> ValueTypeOf<Id> {
            return priv_get<Id>(traits::BoolConstant<IsFieldEnabledV<Id>>());
        }

        template <typename Container, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto pack(IndexSequence<Ids...>) const USTL_NOEXCEPT -> Container {  
            return pack<Container, Ids...>();  
        }

        /// Takes multiple fields and combines them into a single `Int`.
        template <typename Container, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto pack() const USTL_NOEXCEPT -> Container {
            static_assert(traits::IsTrivillyConstructibleV<Container>, "[Error]: ");
            Container out;
            pack_to<Container, Ids...>(out);
            return out;
        }

        /// Takes multiple fields and combines them into a single `Int`.
        template <typename Contaienr, usize Id, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto pack_to(Contaienr &out) const USTL_NOEXCEPT -> void {
            static_assert(sizeof(Contaienr) * ustl::NumericLimits<u8>::DIGITS 
                          >= TotalOccupiedUsizeByIds<FieldList, Id, Ids...>::VALUE, 
                "[Error]: The size of the specified type must be at least as large as the required size.");
            usize const tmp[] = { get<Id>(), get<Ids>()... };

            // TODO(SmallHuaZi) using class template instead of fold expression to provide
            // compatibility for lower version of c++.
            out = Contaienr(get<Id>()) | (((Contaienr(get<Ids>()) << GetFieldBits<FieldList, Id>())) | ...);
        }

        template <typename Container, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto unpack(Container const &container, IndexSequence<Ids...>) const USTL_NOEXCEPT -> void {  
            return unpack<Container, Ids...>(container);
        }
    
        template <typename Container, usize Id, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto unpack(Container const &container) const USTL_NOEXCEPT -> void {
        }

        template <typename Enum>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto operator|(Enum value) -> Self;

    private:
        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto priv_set(ValueTypeOf<Id> const &value, traits::TrueType) USTL_NOEXCEPT -> void {
            USTL_CONSTEXPR 
            usize const idx = FindUsizeIndex<FieldList, Id>();
            values[idx] &= (~GetFieldMask<FieldList, Id>()) | static_cast<usize>(value) << GetFieldShift<FieldList, Id>();
        }

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto priv_set(ValueTypeOf<Id> const &value, traits::FalseType) USTL_NOEXCEPT -> void
        {}

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto priv_get(traits::TrueType) const USTL_NOEXCEPT -> ValueTypeOf<Id> {
            typedef ValueTypeOf<Id> Result;
            USTL_CONSTEXPR 
            usize const idx = FindUsizeIndex<FieldList, Id>();
            return Result((values[idx] & GetFieldMask<FieldList, Id>()) >> GetFieldShift<FieldList, Id>());
        }

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR 
        auto priv_get(traits::FalseType) const USTL_NOEXCEPT -> ValueTypeOf<Id> {
            return ValueTypeOf<Id>();
        }

        USTL_CONSTEXPR
        static usize const NUM_USIZE = bitfields::CalculateOccupiedUsize<FieldList>();
        usize values[NUM_USIZE];
    };
} // namespace bitfields

    USTL_CONSTANT_OPTION(FieldId, usize, ID);
    USTL_CONSTANT_OPTION(FieldBits, usize, BITS);
    USTL_CONSTANT_OPTION(FieldName, util::FixedString, NAME);
    USTL_TYPE_OPTION(FieldType, Type);
    USTL_CONSTANT_OPTION(EnableFieldIf, bool, ENABLE);

    using bitfields::BitField;
    using bitfields::BitFields;

    template <typename Fields>
    struct MakeBitFields;

    template<typename... Fields>
    struct MakeBitFields<TypeList<Fields...>>
    {
        typedef BitFields<Fields...>    Type;  
    };

    template<typename FieldList>
    using MakeBitFieldsT = typename MakeBitFields<FieldList>::Type;

} // namespace ustl

#endif // #ifndef USTL_BITFIELDS_HPP
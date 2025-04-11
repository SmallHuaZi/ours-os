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
#include <ustl/util/visit.hpp>
#include <ustl/util/fold.hpp>
#include <ustl/traits/void.hpp>
#include <ustl/util/find_if.hpp>
#include <ustl/util/type-list.hpp>
#include <ustl/util/fixed_string.hpp>
#include <ustl/util/index_sequence.hpp>
#include <ustl/util/pack_options.hpp>
#include <ustl/views/span.hpp>
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
    /// Not be used now
    enum class IdPolicy {
        Auto,
        Manually,
    };

    USTL_CONSTEXPR
    static auto const ID_NONE = ustl::NumericLimits<usize>::DIGITS;

    USTL_CONSTEXPR
    static auto const START_BIT_NONE = ustl::NumericLimits<usize>::DIGITS;

    /// Manually specifies the ID of a field. If not provided, it defaults to 
    /// an auto-incrementing strategy.
    USTL_CONSTANT_OPTION(Id, usize, ID);

    /// Indicates how many bits the field needs.
    USTL_CONSTANT_OPTION(Bits, usize, BITS);

    /// Explicitly specify the start bit of a field. Please use with caution, 
    /// as there is a high probability of overlap with other fields.
    USTL_CONSTANT_OPTION(StartBit, usize, START_BIT);

    /// The name of field used for pretty printting.
    USTL_CONSTANT_OPTION(Name, util::FixedString, NAME);

    /// Enable or disable the field. If a field is disabled, it will not consume 
    /// memory, and any operations applied to it will have no effect.
    USTL_CONSTANT_OPTION(Enable, bool, ENABLE);

    /// Specify the storage type of the field. This offers convenience by reducing 
    /// the need to explicitly provide the ID when invoking get and set methods.
    USTL_TYPE_OPTION(Type, ValueType);

    /// Metadata for a single field.
    template<typename StorageUnit, int Id, int StartBit, int Bits, typename Type, util::FixedString Name, bool Enable>
    struct FieldMetaData {
        USTL_CONSTEXPR
        static auto const IS_TEMPORARY = traits::IsVoidV<StorageUnit>;
        typedef traits::ConditionalT<IS_TEMPORARY, usize, StorageUnit>   StorageUnitType;

        /// The following is interface members.

        typedef Type    ValueType;

        USTL_CONSTEXPR
        static auto const ID = Id;

        USTL_CONSTEXPR
        static auto const BITS = Bits;

        // Validate the range of filed .
        static_assert(BITS <= ustl::NumericLimits<StorageUnitType>::DIGITS,
                     "[ustl-error]: The field can not be cross multiple units");

        USTL_CONSTEXPR
        static StorageUnitType const START_BIT = StartBit;

        USTL_CONSTEXPR
        static StorageUnitType const SHIFT = IS_TEMPORARY ? 0 : (START_BIT % ustl::NumericLimits<StorageUnitType>::DIGITS);

        // Validate the range of filed .
        static_assert(SHIFT + BITS <= ustl::NumericLimits<StorageUnitType>::DIGITS,
                     "[ustl-error]: The field can not be cross multiple units");

        USTL_CONSTEXPR
        static StorageUnitType const MASK = IS_TEMPORARY ? 0 : MakeBitMask<StorageUnitType, SHIFT, BITS>::VALUE;

        USTL_CONSTEXPR
        static auto const UNIT = IS_TEMPORARY ? 0 : StartBit / ustl::NumericLimits<StorageUnitType>::DIGITS;

        USTL_CONSTEXPR
        static auto const NAME = Name;

        USTL_CONSTEXPR
        static auto const ENABLE = Enable;
    };

    template <int Bits>
    struct SkipBit {
        USTL_CONSTEXPR
        static auto const ID = ustl::NumericLimits<int>::max();
    };

    struct DefaultFieldMetaData {
        typedef usize   ValueType;
        typedef void    StorageUnitType;

        USTL_CONSTEXPR
        static auto const BITS = 1;

        USTL_CONSTEXPR
        static auto const START_BIT = START_BIT_NONE;

        USTL_CONSTEXPR
        static auto const ENABLE = true;

        USTL_CONSTEXPR
        static util::FixedString const NAME{"Anonymous"};
    };

    /// Make a metadata block from the given options.
    ///
    /// `Default` is namely `DefaultFieldMetaData` above.
    ///
    /// `Unnormalized` refers to a field provided by the user through Field<..>. 
    /// It is an incomplete field lacking certain meta-information, requiring the 
    /// internal library to fill in the missing details.
    template <typename DefaultOrUnnormalized, typename... Options>
    struct MakeBitField {
        typedef typename PackOptions<DefaultOrUnnormalized, Options...>::Type
            PackedOption;

        typedef FieldMetaData<
            typename PackedOption::StorageUnitType,
            PackedOption::ID,
            PackedOption::START_BIT,
            PackedOption::BITS,
            typename PackedOption::ValueType,
            PackedOption::NAME,
            PackedOption::ENABLE
        > Type;
    };

    /// Export it for user
    template <typename... Options>
    using Field = typename MakeBitField<DefaultFieldMetaData, Options...>::Type;

    /// Relocate all fields.
    template <typename Storage, typename FieldList, int BitAcc = 0>
    struct RelocateFields;

    template <typename Storage, int BitAcc>
    struct RelocateFields<Storage, TypeList<>, BitAcc> {
        typedef TypeList<>  Type;
    };

    /// Internal auxiliary option.
    USTL_TYPE_OPTION(StorageUnit, StorageUnitType);

    /// This field is valid, now we must relocate it's position and store it to
    /// the final list of field.
    template <typename Storage, int BitAcc, typename HeadField, typename... TailFields>
    struct RelocateFields<Storage, TypeList<HeadField, TailFields...>, BitAcc> {
        USTL_CONSTEXPR
        static auto const WAS_START_BIT_GIVEN = HeadField::START_BIT != START_BIT_NONE;

        // Validate the given start bit
        static_assert(!WAS_START_BIT_GIVEN || HeadField::START_BIT >= BitAcc,
                      "[ustl-error]: The start bit of a field specified manually "
                      "by user must not overlap with other fields");

        USTL_CONSTEXPR
        static auto const START_BIT = WAS_START_BIT_GIVEN ? HeadField::START_BIT : BitAcc;

        // Do recursion to get the result from sub-branch.
        typedef typename RelocateFields<Storage, TypeList<TailFields...>, START_BIT + HeadField::BITS>::Type
            NewFieldListWithoutHeadField;
        // Then we first fix the head field up.
        typedef typename MakeBitField<HeadField, StorageUnit<Storage>, StartBit<START_BIT>>::Type
            HeadFieldRelocated;
        // Merge results from sub-branch and this node then to return it to parent node.
        typedef TypeAlgos::PushFront<NewFieldListWithoutHeadField, HeadFieldRelocated>
            Type;
    };

    /// Handle a null field.
    template <typename Storage, int BitAcc, int SkippedBits, typename... TailFields>
    struct RelocateFields<Storage, TypeList<SkipBit<SkippedBits>, TailFields...>, BitAcc> {
        typedef typename RelocateFields<Storage, TypeList<TailFields...>, BitAcc + SkippedBits>::Type
            Type;
    };

    template <typename FieldList>
    struct FilterDisabledFields;

    template <>
    struct FilterDisabledFields<TypeList<>> {
        typedef TypeList<>  Type;
    };

    template <typename HeadField, typename... Fields>
    struct FilterDisabledFields<TypeList<HeadField, Fields...>> {
        typedef typename FilterDisabledFields<TypeList<Fields...>>::Type NewFieldListWithoutHeadField;
        typedef traits::ConditionalT<HeadField::ENABLE, 
            TypeAlgos::PushFront<NewFieldListWithoutHeadField, HeadField>,
            NewFieldListWithoutHeadField
        > Type;
    };

    template <int BitsSkipped, typename... Fields>
    struct FilterDisabledFields<TypeList<SkipBit<BitsSkipped>, Fields...>> {
        typedef typename FilterDisabledFields<TypeList<Fields...>>::Type NewFieldListWithoutHeadField;
        typedef TypeAlgos::PushFront<NewFieldListWithoutHeadField, SkipBit<BitsSkipped>> Type;
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
        struct HasSameId
            : public traits::BoolConstant<HeadField::ID == Field::ID>
        {  typedef bool    RetType;  };

        template <int BitsSkipped>
        struct HasSameId<SkipBit<BitsSkipped>>
            : public traits::FalseType
        {  typedef bool    RetType;  };

        typedef HasDuplicates<TypeList<RemainingFields...>>   NextIteration;

        USTL_CONSTEXPR
        static bool const VALUE = {
            util::Fold<HasSameId, util::Accumulator::BoolOr, RemainingFields...>::VALUE ||
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

    /// TotalOccupiedBits
    template <typename FieldList>
    struct TotalOccupiedBits;

    template <>
    struct TotalOccupiedBits<TypeList<>>
        : public traits::IntConstant<0>
    {};

    template <typename... Fields>
    struct TotalOccupiedBits<TypeList<Fields...>>
        : public traits::IntegralConstantInterface<TotalOccupiedBits<TypeList<Fields...>>, usize>
    {
        template <typename Field>
        struct GetOccupiedBits
            : public traits::IntConstant<Field::BITS>
        {  typedef isize    RetType;  };

        template <int BitsSkipped>
        struct GetOccupiedBits<SkipBit<BitsSkipped>>
            : public traits::IntConstant<BitsSkipped>
        {  typedef isize    RetType;  };

        USTL_CONSTEXPR
        static usize const VALUE = {
            util::Fold<GetOccupiedBits, util::Accumulator::IntAdd, Fields...>::VALUE
        };
    };

    template <typename Unit, typename FieldList>
    struct CalculateOccupiedUnits;

    template <typename Unit, typename... Fields>
    struct CalculateOccupiedUnits<Unit, TypeList<Fields...>>
        : public traits::IntegralConstantInterface<CalculateOccupiedUnits<Unit, TypeList<Fields...>>, usize>
    {
        typedef TypeList<Fields...>     FieldList;

        USTL_CONSTEXPR
        static usize const BITS = ustl::NumericLimits<Unit>::DIGITS;

        typedef ustl::traits::ConditionalT<sizeof...(Fields) == 0,
            traits::IntConstant<1>,
            TotalOccupiedBits<FieldList>>   GetOccupiedBits;

        USTL_CONSTEXPR
        static usize const VALUE = (GetOccupiedBits() + BITS - 1) / BITS; // align up
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

    template <int TotalBits>
    struct GetStorageType {
        typedef traits::ConditionalT<TotalBits < sizeof(u8) * 8, u8,
                traits::ConditionalT<TotalBits < sizeof(u16) * 8, u16,
                traits::ConditionalT<TotalBits < sizeof(u32) * 8, u32,
                //traits::ConditionalT<TotalBits < sizeof(u64) * 8, u64, 
                //traits::ConditionalT<TotalBits < sizeof(u128) * 8, u128,
                usize // By default, we use `usize` but `u64`.
            >>> Type;
    };

    /// FIXME(SmallHuaZi) This feature is still in the experimental phase, and there are many
    /// scenarios we have not yet accounted for, such as alignment storage, fields spanning multiple
    /// usizes, etc. Please use it with caution.
    ///
    /// TODO(SmallHuaZi)
    ///      1. Implement serialzation to support pretty print.
    ///      2. Provide reference to fields, namely BitFieldRef<>
    template<typename StorageUnitT, typename FieldListT>
    struct BitFieldsImpl {
        typedef BitFieldsImpl       Self;
        typedef StorageUnitT        StorageUnit; 
        typedef FieldListT          FieldList;

    public:
        USTL_CONSTEXPR
        static usize const NUM_BITS = TotalOccupiedBits<FieldList>();

        USTL_CONSTEXPR
        static usize const NUM_UNITS = CalculateOccupiedUnits<StorageUnit, FieldList>();

        template <usize Id>
        USTL_CONSTEXPR
        static auto const IsFieldEnabledV = GetFieldT<FieldList, Id>::ENABLE;

        template <usize Id>
        using ValueTypeOf = typename GetFieldT<FieldList, Id>::ValueType;

        USTL_CONSTEXPR
        BitFieldsImpl() = default;

        USTL_CONSTEXPR
        BitFieldsImpl(ustl::InitializerList<usize> const &val) {
            ustl::algorithms::copy_n(val.begin(), NUM_UNITS > val.size() ? val.size() : NUM_UNITS, values);
        }

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto set(ValueTypeOf<Id> const &value) USTL_NOEXCEPT -> Self & {
            priv_set<Id>(value, traits::BoolConstant<IsFieldEnabledV<Id>>());
            return *this;
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

        /// Takes multiple fields and combines them into a single `Layout`.
        template <typename Layout, usize Id, usize... Ids>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto pack_to(Layout &out) const USTL_NOEXCEPT -> void {
            static_assert(sizeof(Layout) * 8 >= TotalOccupiedUsizeByIds<FieldList, Id, Ids...>::VALUE,
                          "[ustl-error]: The size of the specified type must be at least as large as the required size.");
            typedef IndexSequence<0, GetFieldT<FieldList, Ids>::BITS...>     BitsOfAllField;
            typedef IntegerSequenceCalculatePresumT<BitsOfAllField>         ShiftsOnFlatLayout;

            // TODO(SmallHuaZi) using class template instead of fold expression to provide
            // compatibility for older version of c++.
            out = Layout(get<Id>()) | (((Layout(get<Ids>()) << GetFieldT<FieldList, Id>::BITS)) | ...);
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
            // Defining the variables is just to observe their value. Usually them will
            // be optimized out. So don't worry that them occupy the statck space.
            USTL_CONSTEXPR usize const idx = GetFieldT<FieldList, Id>::UNIT;
            USTL_CONSTEXPR usize const mask = GetFieldT<FieldList, Id>::MASK;
            USTL_CONSTEXPR usize const shift = GetFieldT<FieldList, Id>::SHIFT;

            values[idx] &= ~mask;
            values[idx] |= usize(value) << shift;
        }

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto priv_set(ValueTypeOf<Id> const &value, traits::FalseType) USTL_NOEXCEPT -> void
        {}

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto priv_get(traits::TrueType) const USTL_NOEXCEPT -> ValueTypeOf<Id> {
            typedef ValueTypeOf<Id> Result;
            USTL_CONSTEXPR usize const idx = GetFieldT<FieldList, Id>::UNIT;
            USTL_CONSTEXPR usize const mask = GetFieldT<FieldList, Id>::MASK;
            USTL_CONSTEXPR usize const shift = GetFieldT<FieldList, Id>::SHIFT;
            return Result((values[idx] & mask) >> shift);
        }

        template<usize Id>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto priv_get(traits::FalseType) const USTL_NOEXCEPT -> ValueTypeOf<Id> {
            // TODO (SmallHuaZi): Handle the scenario where a field is disabled as an optional configuration item.
            //  1. Strict Constraint: Treat it as a hard error at compile time.
            //  2. Flexible Mode: Return a default value to the caller, supporting scenarios where a flag is only 
            //     applicable on 64-bit platforms but defaults to zero on 32-bit platforms. It is frequently seen.
            return ValueTypeOf<Id>();
        }

        template <typename Layout, usize... Id>
        USTL_FORCEINLINE USTL_CONSTEXPR
        auto priv_pack_to(Layout &out, IndexSequence<Id...>) const USTL_NOEXCEPT -> void {
            
        }

        StorageUnit values[NUM_UNITS];
    };

    template <typename... Fields>
    struct MakeBitFields {
        typedef typename MakeBitFields<TypeList<Fields...>>::Type    Type;
    };

    template <typename... Fields>
    struct MakeBitFields<TypeList<Fields...>> {
        typedef TypeList<Fields...>     RawFieldList;
        static_assert(!bitfields::HasDuplicates<RawFieldList>::VALUE, "Has duplicated fields");

        typedef typename FilterDisabledFields<RawFieldList>::Type  FieldListFiltered;

        USTL_CONSTEXPR 
        static usize const NUM_BITS = TotalOccupiedBits<FieldListFiltered>();

        typedef typename GetStorageType<NUM_BITS>::Type   StorageUnit;
        typedef typename RelocateFields<StorageUnit, FieldListFiltered>::Type  FieldList;
        typedef BitFieldsImpl<StorageUnit, FieldList>    Type;
    };

    template <typename T, typename... Fields>
    struct MakeBitFields<StorageUnit<T>, TypeList<Fields...>> {
        typedef TypeList<Fields...>     RawFieldList;
        static_assert(!bitfields::HasDuplicates<RawFieldList>::VALUE, "Has duplicated fields");

        typedef typename FilterDisabledFields<RawFieldList>::Type  FieldListFiltered;

        USTL_CONSTEXPR 
        static usize const NUM_BITS = TotalOccupiedBits<FieldListFiltered>();

        typedef T   StorageUnit;
        typedef typename RelocateFields<StorageUnit, FieldListFiltered>::Type  FieldList;
        typedef BitFieldsImpl<StorageUnit, FieldList>    Type;
    };

    template <typename... Fields>
    using BitFields = typename MakeBitFields<Fields...>::Type;

    /// UNUSED: This option is used to support `operator|[=]` and `operator&[=]` for some enum fields.
    /// It is very optional, please ensure only one field specified the enum type, otherwise causing
    /// a static assertion error.
    USTL_CONSTANT_OPTION(EnableBitOpForEnumField, bool, ENABLE_BITOP_FOR_ENUM_FIELD);

} // namespace bitfields
    using bitfields::Field;
    using bitfields::BitFields;

    template <typename... Fields>
    struct SizeOf<bitfields::BitFieldsImpl<Fields...>>
        : public traits::IntConstant<sizeof...(Fields)>
    {};

    /// TODO(SmallHuaZi) Create a Map<Index, Id> at compile time to avoid the assumption
    /// that requires `Id` to be forcibly identical to `Index`.
    /// Note that this map does not refer to the collection `ustl::collections::Map`.
    template <usize I, typename... Fields>
    struct Getter<bitfields::BitFieldsImpl<Fields...>, I> {
    };

} // namespace ustl

#endif // #ifndef USTL_BITFIELDS_HPP
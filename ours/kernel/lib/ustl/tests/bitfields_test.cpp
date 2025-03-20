#include <ustl/bitfields.hpp>
#include <bit>
#include <gtest/gtest.h>

using namespace ustl;
using namespace ustl::bitfields;

enum FieldId {
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
};


typedef BitFields<
    Field<Id<Zero>, Name<"Zero">>,
    Field<Id<One>, Name<"One">>,
    Field<Id<Two>, Name<"Two">>,
    Field<Id<Three>, Name<"Three">>,
    Field<Id<Four>, Name<"Four">>,
    Field<Id<Five>, Name<"Five">>,
    Field<Id<Six>, Name<"Six">>,
    Field<Id<Seven>, Name<"Seven">>,
    Field<Id<Eight>, Name<"Eight">>,
    Field<Id<Nine>, Name<"Nine">>,
    Field<Id<Ten>, Name<"Ten">>
> SingleBitDomain;
static_assert(sizeof(SingleBitDomain) == sizeof(u16), "");

TEST(BitFields, SingleBitDomain) {
    union Helper {
        SingleBitDomain domain;
        u16 value;
        struct {
            u16 _0: 1;
            u16 _1: 1;
            u16 _2: 1;
            u16 _3: 1;
            u16 _4: 1;
            u16 _5: 1;
            u16 _6: 1;
            u16 _7: 1;
            u16 _8: 1;
            u16 _9: 1;
            u16 _10: 1;
        };
    };

    Helper helper;
    memset(&helper, 0, sizeof(helper));

    u16 value = 0;

#define SET_AND_CLEAR(I) \
    value = helper.value;\
    helper.domain.set<I>(1); \
    ASSERT_EQ(helper._##I, 1); \
    ASSERT_EQ(value, helper.value & (~(u16(1) << u16(I))));\
    helper.domain.set<I>(0); \
    ASSERT_EQ(helper._##I, 0); \
    ASSERT_EQ(value, helper.value);

    SET_AND_CLEAR(1);
    SET_AND_CLEAR(2);
    SET_AND_CLEAR(3);
    SET_AND_CLEAR(4);
    SET_AND_CLEAR(5);
    SET_AND_CLEAR(6);
    SET_AND_CLEAR(7);
    SET_AND_CLEAR(8);
    SET_AND_CLEAR(9);
    SET_AND_CLEAR(10);
}

template <bool EnableSecond, bool EnableThird, bool EnableFourth, bool EnableSkip>
using MultiBitFieldList = BitFields<
    Field<Id<Nine>, Name<"Nine">, Bits<2>>,
    Field<Id<Ten>, Name<"Ten">, Bits<10>, Enable<EnableSecond>>,
    Field<Id<One>, Name<"One">, Bits<30>, Enable<EnableThird>>,
    SkipBit<EnableSkip ? 23 : 0>,
    Field<Id<Two>, Name<"Two">, Bits<32>, Enable<EnableFourth>>
>;
static_assert(sizeof(MultiBitFieldList<false, false, false, false>) == 1);
typedef MultiBitFieldList<true, false, false, false> TFFF;
static_assert(sizeof(TFFF) == 2);
typedef MultiBitFieldList<true, true, false, false> TTFF;
static_assert(sizeof(TTFF) == 8);
typedef MultiBitFieldList<true, true, true, true>   TTTT;
static_assert(sizeof(TTTT) == 16);

TEST(bitfields, MultiBitDomain) {
    u32 const initvals[] = { 3, 888, 77499, 66883 };
    TTTT tttt;
    memset(&tttt, 0, sizeof(tttt));

    tttt.set<Nine>(initvals[0]);
    ASSERT_EQ(tttt.get<Nine>(), initvals[0]);
    ASSERT_EQ(tttt.get<Ten>(), 0);
    ASSERT_EQ(tttt.get<One>(), 0);
    ASSERT_EQ(tttt.get<Two>(), 0);

    tttt.set<Ten>(initvals[1]);
    ASSERT_EQ(tttt.get<Ten>(), initvals[1]);
    ASSERT_EQ(tttt.get<Nine>(), initvals[0]);
    ASSERT_EQ(tttt.get<Two>(), 0);
    ASSERT_EQ(tttt.get<One>(), 0);

    tttt.set<One>(initvals[2]);
    ASSERT_EQ(tttt.get<One>(), initvals[2]);
    ASSERT_EQ(tttt.get<Ten>(), initvals[1]);
    ASSERT_EQ(tttt.get<Nine>(), initvals[0]);
    ASSERT_EQ(tttt.get<Two>(), 0);

    tttt.set<Two>(initvals[3]);
    ASSERT_EQ(tttt.get<Two>(), initvals[3]);
    ASSERT_EQ(tttt.get<One>(), initvals[2]);
    ASSERT_EQ(tttt.get<Ten>(), initvals[1]);
    ASSERT_EQ(tttt.get<Nine>(), initvals[0]);
}
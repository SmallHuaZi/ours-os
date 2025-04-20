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
#ifndef OURS_TESTS_TEST_HPP
#define OURS_TESTS_TEST_HPP 1

#include <ours/init.hpp>
#include <ours/assert.hpp>
#include <ours/macro_abi.hpp>

#include <ustl/source_location.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/collections/intrusive/set.hpp>

#define OTEST_ABI   INIT_SECTION("unittests")

#define OTEST(TEST_SUITE, TEST_CASE) \
    class OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) \
        : public ::ours::test::TEST_SUITE {\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)() = default;\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)(\
            OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) const &\
        ) = default;\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)(\
            OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) &&\
        ) = default;\
        virtual ~OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)() = default;\
        static ::ours::test::TestInfo *s_test_info_ptr;\
        static ::ours::test::TestInfo  s_test_info;\
        virtual auto execute() -> void override;\
    };\
    ::ours::test::TestInfo OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::*s_test_info_ptr = \
        entrol_test(\
            &OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::s_test_info,\
            #TEST_SUITE,\
            #TEST_CASE\
        );\
    auto OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::execute() -> void

#define OTEST_INPLACE(TEST_SUITE, TEST_CASE) \
    class OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) \
        : public ::ours::test::TEST_SUITE {\
        typedef CommonTestFactoryInpace<OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)>    Factory;\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)() = default;\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)(\
            OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) const &\
        ) = default;\
        OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)(\
            OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE) &&\
        ) = default;\
        virtual ~OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)() = default;\
        static ::ours::test::TestInfo *s_test_info_ptr;\
        static ::ours::test::TestInfoInplace<OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)Holder>  s_test_info;\
        virtual auto execute() -> void override;\
    };\
    ::ours::test::TestInfo OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::*s_test_info_ptr = \
        entrol_test(\
            &OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::s_test_info,\
            #TEST_SUITE,\
            #TEST_CASE\
            CommonTestFactoryInpace<OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)>()\
        );\
    auto OURS_MPL_CONTACT(TEST_SUITE, TEST_CASE)::execute() -> void

#define ASSERT_TRUE(CONDITION, ...) \
    ASSERT(CONDITION, __VA_ARGS__)  // Ours internal assertion routine (Unstable)

#define ASSERT_FALSE(CONDITION, ...) \
    ASSERT((!CONDITION), __VA_ARGS__)  // Ours internal assertion routine (Unstable)

namespace ours::test {
    // struct TestSuite;

    // struct Test {
    //     Test() = default;
    //     virtual ~Test() = default;

    //     virtual auto setup() -> void {};
    //     virtual auto teardown() -> void {};
    //     virtual auto execute() -> void = 0;

    //     auto run() -> void;
    // };

    // struct ITestFactory {
    //     virtual auto create_test() -> Test * = 0;
    // };

    // template <typename T>
    // struct CommonTestFactory: public ITestFactory {
    //     virtual auto create_test() -> T * override {
    //         return new T();
    //     }
    // };

    // template <typename T>
    // struct CommonTestFactoryInpace: public ITestFactory {
    //     virtual auto create_test() -> Test * override {
    //         static T object;
    //         ustl::mem::construct_at(&object);
    //         return &object;
    //     }
    // };

    // struct TestInfo: public ustl::collections::intrusive::SetBaseHook<> {
    //     TestInfo() = default;
    //     TestInfo(char const *suite_name, char const *test_name, 
    //              ustl::SourceLocation location, ITestFactory *factory)
    //     {}

    //     char const *test_name;
    //     char const *suite_name;
    //     ustl::SourceLocation location;
    //     ITestFactory *factory_;
    // };

    // template <typename Test>
    // struct TestInfoInplace: TestInfo {
    //     typedef TestInfoInplace     Self;
    //     typedef TestInfo            Base;
    //     typedef CommonTestFactoryInpace<Test>   TestFactory;

    //     TestInfoInplace() = default;
    //     TestInfoInplace(char const *suite_name, char const *test_name, 
    //                     ustl::SourceLocation location, TestFactory factory)
    //         : Base(suite_name, test_name, location, &factory_),
    //           factory_(factory)
    //     {}

    //     TestFactory factory_;
    // };

    // template <typename Factory>
    // TestInfoInplace(char const *, char const *, ustl::SourceLocation, Factory) -> TestInfoInplace<Factory>;

    // struct UnitTest {
    //     typedef UnitTest    Self;
    //     static auto get_instance() -> Self * {
    //         // The lazy initialization mechanism will help us establish the correct sequence
    //         // about the construction of test objects.
    //         static Self s_unittest;
    //         return &s_unittest;
    //     }

    //     auto add_test_info(TestInfo *ti) 
    //         -> TestInfo * {
    //         DEBUG_ASSERT(ti != nullptr);
    //         test_info_list_.insert(*ti);
    //         return ti;
    //     }

    //     ustl::collections::intrusive::MultiSet<TestInfo> test_info_list_;
    // };

    // template <typename TestInfo, typename Factory>
    // auto entrol_test(TestInfo *ti, char const *suite_name, char const *test_name,
    //                  Factory factory, ustl::SourceLocation sl = ustl::SourceLocation::current()) 
    //      -> TestInfo * 
    // {
    //     DEBUG_ASSERT(ti != nullptr);
    //     ustl::mem::construct_at(ti, suite_name, test_name, factory, sl);
    //     UnitTest::get_instance()->add_test_info(ti);
    // }

} // namespace ours::test

#endif // #ifndef OURS_TESTS_TEST_HPP
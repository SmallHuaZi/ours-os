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
#ifndef OURS_TESTS_INIT_TEST_HPP
#define OURS_TESTS_INIT_TEST_HPP 1

#include <ours/tests/test.hpp>

#include <gktl/init_hook.hpp>
#include <ustl/traits/conditional.hpp>
#include <ustl/traits/cv.hpp>
#include <ustl/traits/ref.hpp>

#define OTEST_INIT(InitLevel, SUITE_NAME, TEST_NAME)

namespace ours::test {
    typedef gktl::InitLevel     InitState;

    template <typename State>
    struct StateObserver {
        CXX11_CONSTEXPR
        static auto const kMaxInpalceStateSize = 32;

        typedef ustl::traits::RemoveCvRefT<State>   RawStateType;
        typedef ustl::traits::ConditionalT<(sizeof(RawStateType) > kMaxInpalceStateSize),
            RawStateType, 
            ustl::traits::AddLvalRefT<ustl::traits::AddConstT<RawStateType>>
        > StateType;

        virtual auto current_state() -> StateType = 0;
    };

    template <typename State>
    struct StateTestFixture: public Test {
    };

    template <typename State>
    struct StateTestInfo: TestInfoInplace<StateTestFixture<State>> {
        typedef StateObserver<State>    Observer;
    }; 

} // namespace ours::tests

#endif // #ifndef OURS_TESTS_INIT_TEST_HPP
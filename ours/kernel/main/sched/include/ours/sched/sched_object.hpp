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

#ifndef OURS_SCHED_SCHED_OBJECT_HPP
#define OURS_SCHED_SCHED_OBJECT_HPP

#include <ours/cpu-mask.hpp>
#include <ours/sched/sched_states.hpp>

#include <ustl/collections/intrusive/any_hook.hpp>

namespace ours::sched {
    class Scheduler;

    // Classes `Process`, `Thread` and `SchedGroup` Derived from this class.
    class SchedObject {
        typedef SchedObject         Self;
    public:
        // auto state() -> State
        // {  return this->_state;  }

        // auto set_state(State state) -> void 
        // {  this->_state = state;  }

        auto get_scheduler() -> Scheduler *
        {  return this->scheduler_;  }

        auto set_scheduler(Scheduler *scheduler) -> void
        {  this->scheduler_ = scheduler;  }

        static auto current() -> Self *;

        // friend auto operator<(Self const &x, Self const &y) -> bool
        // {  return x._weight < y._weight;  }

    protected:
        friend Scheduler;

        Self        *parent_;
        Scheduler   *scheduler_;
        SchedTime    runtime_;
        CpuMask      cpumask_;
        // State        _state;
        // LoadWeight   _weight;
        ustl::collections::intrusive::AnyMemberHook<> managed_hook_;

    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

} // namespace ours::sched

#define SCHED_OBJECT_INTERFACE

#endif // #ifndef OURS_CORE_TASK_SCHED_OBJECT_HPP
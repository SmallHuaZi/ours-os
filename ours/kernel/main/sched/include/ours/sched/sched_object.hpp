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
#include <ours/sched/types.hpp>

#include <ustl/collections/intrusive/any_hook.hpp>

namespace ours::sched {
    class SchedObject {
        typedef SchedObject         Self;
      public:
        FORCE_INLINE
        auto deadline() const -> SchedTime {
            return deadline_;
        }

        FORCE_INLINE
        auto vruntime() const -> SchedTime {
            return vruntime_;
        }

        /// Update the object's deadline.
        ///
        /// Return true if current object has consumed its request.
        FORCE_INLINE
        auto update(SchedTime delta) -> bool {
            if (vruntime_ >= deadline_) {
                return true;
            }
            vruntime_ += delta;
            return false;
        }
      protected:
        friend class Scheduler;

        FORCE_INLINE
        auto get_scheduler() -> Scheduler * {
            return this->scheduler_;
        }

        FORCE_INLINE
        auto set_scheduler(Scheduler *scheduler) -> void {
            this->scheduler_ = scheduler;
        }

        SchedTime deadline_;
        SchedTime vruntime_;
        SchedTime time_slice_;
        SchedWeight weight_;
        Scheduler *scheduler_;
        ustl::collections::intrusive::AnyMemberHook<> managed_hook_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

} // namespace ours::sched

#define SCHED_OBJECT_INTERFACE

#endif // #ifndef OURS_CORE_TASK_SCHED_OBJECT_HPP
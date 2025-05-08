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

#include <ustl/sync/atomic.hpp>
#include <ustl/collections/intrusive/any_hook.hpp>

#include <cnl/fraction.hpp>

namespace ours::sched {
    CXX11_CONSTEXPR
    static SchedWeight const kPriorityToWeightMap[] = {
        121,   149,   182,   223,   273,   335,   410,   503,   616,   754,  924,
        1132,  1386,  1698,  2080,  2549,  3122,  3825,  4685,  5739,  7030, 8612,
        10550, 12924, 15832, 19394, 23757, 29103, 35651, 43672, 53499, 65536
    };

    CXX11_CONSTEXPR
    static SchedWeight const kMinWeight(kPriorityToWeightMap[0]);

    FORCE_INLINE CXX11_CONSTEXPR 
    auto priority_to_weight(usize priority) -> SchedWeight {
        DEBUG_ASSERT(priority < std::size(kPriorityToWeightMap));
        return kPriorityToWeightMap[priority];
    }

    enum class SchedDiscipline {
        Eevdf,
    };

    struct BaseProfile {
        BaseProfile() = default;

        explicit BaseProfile(usize priority)
            : weight(priority_to_weight(priority)),
              discipline(SchedDiscipline::Eevdf)
        {}

        SchedWeight weight;
        SchedDiscipline discipline;
    };

    class PreemptionState {
      public:
        auto is_pending() const -> bool;

        auto set_pending() -> void;

        auto clear_pending() -> void;
      private:
        ustl::sync::Atomic<CpuMask> pending_;
    };

    class SchedObject {
        typedef SchedObject         Self;
      public:
        SchedObject() = default;

        SchedObject(BaseProfile profile)
            : profile_(profile)
        {}

        FORCE_INLINE
        auto deadline() const -> SchedTime {
            return deadline_;
        }

        FORCE_INLINE
        auto vruntime() const -> SchedTime {
            return vruntime_;
        }

        FORCE_INLINE
        auto weight() const -> SchedWeight {
            return profile_.weight;
        }

        FORCE_INLINE
        auto preemption_state() -> PreemptionState & {
            return preemption_state_;
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
        friend class MainScheduler;
        friend class EevdfScheduler;

        FORCE_INLINE
        auto get_scheduler() -> IScheduler * {
            return this->scheduler_;
        }

        FORCE_INLINE
        auto set_scheduler(IScheduler *scheduler) -> void {
            this->scheduler_ = scheduler;
        }

        BaseProfile profile_;
        PreemptionState preemption_state_;

        /// The start time point of execution.
        SchedTime runtime_;  // Physical time

        SchedTime deadline_; // Virtual time
        SchedTime vruntime_; // Virtual time
        SchedTime time_slice_;
        IScheduler *scheduler_;
        ustl::collections::intrusive::AnyMemberHook<> managed_hook_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

} // namespace ours::sched

#define SCHED_OBJECT_INTERFACE

#endif // #ifndef OURS_CORE_TASK_SCHED_OBJECT_HPP
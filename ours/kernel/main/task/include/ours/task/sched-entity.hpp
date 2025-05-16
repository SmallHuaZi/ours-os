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
#ifndef OURS_TASK_SCHED_STATE_HPP
#define OURS_TASK_SCHED_STATE_HPP

#include <ours/cpu-mask.hpp>
#include <ours/cpu-local.hpp>
#include <ours/task/types.hpp>

#include <ustl/chrono/duration.hpp>
#include <ustl/sync/atomic.hpp>
#include <ustl/collections/intrusive/any_hook.hpp>

// #include <cnl/fraction.hpp>

namespace ours::task {
    using ustl::chrono::Nanoseconds;
    using ustl::chrono::Microseconds;
    using ustl::chrono::Milliseconds;

    typedef ustl::chrono::Nanoseconds   SchedTime;
    typedef ustl::chrono::Nanoseconds   SchedDuration;

    /// High weight accompanied by low virtual time flow rate.
    ///
    /// The unit convertion between real time and virtual as following:
    ///     V(t) = t / TotalWeight;
    ///     t = V(t) * TotalWeight;
    ///
    /// For a task, 
    /// define S(i) as the service time obtained by the task i
    /// define Ve(i) = the start time point of S(i) and the eligible time in EEVDF
    /// define Vd(i) = the end time point of S(i)
    ///     S(i) = (Vd(i) - Ve(i)) * W(i);
    ///     Vd(i) = Ve(i) + Si(i) / W(i)
    // typedef cnl::Fraction<usize, cnl::Power<16>>    SchedWeight;
    typedef usize SchedWeight;

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

    enum class SchedAlgorithm {
        Fair,
    };

    struct BaseProfile {
        BaseProfile() = default;

        explicit BaseProfile(usize priority)
            : weight(priority_to_weight(priority)),
              algorithm(SchedAlgorithm::Fair)
        {}

        SchedWeight weight;
        SchedAlgorithm algorithm;
    };

    class PreemptionState {
      public:
        FORCE_INLINE
        auto is_preemptible() const -> bool {
            return !preemption_disabled_count_ &&
                   !eager_preemption_disabled_count_;
        }

        auto reenable_eager_preemption() -> void;

        FORCE_INLINE
        auto enable_preemption() -> void {
            preemption_disabled_count_.fetch_sub(1);
        }

        FORCE_INLINE
        auto disable_preemption() -> void {
            preemption_disabled_count_.fetch_add(1);
        }

        FORCE_INLINE
        auto set_pending(CpuMask mask = CpuMask::from_cpu_num(CpuLocal::cpunum())) -> void {
            pending_.store(pending_.load() | mask);
        }

        FORCE_INLINE
        auto clear_pending() -> void {
            pending_.store(CpuMask());
        }
      private:
        ustl::sync::AtomicU16 preemption_disabled_count_;
        ustl::sync::AtomicU16 eager_preemption_disabled_count_;
        ustl::sync::Atomic<CpuMask> pending_;
    };

    class SchedEntity {
        typedef SchedEntity         Self;
      public:
        SchedEntity() = default;

        SchedEntity(BaseProfile profile)
            : profile_(profile),
              managed_hook_()
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
        auto vlag() const -> SchedTime {
            return deadline_ - vruntime_;
        }

        FORCE_INLINE
        auto weight() const -> SchedWeight {
            return profile_.weight;
        }

        FORCE_INLINE
        auto preemption_state() -> PreemptionState & {
            return preemption_state_;
        }

        FORCE_INLINE
        auto get_available_mask(CpuMask mask) const -> CpuMask {
            return affinity_mask_ & mask;
        }

        FORCE_INLINE
        auto recent_cpu() const -> CpuNum {
            return recent_cpu_;
        }

        FORCE_INLINE
        auto current_cpu() const -> CpuNum {
            return current_cpu_;
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
        friend class FairScheduler;

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

        bool on_queue_;
        CpuNum recent_cpu_;
        CpuNum current_cpu_;
        CpuMask affinity_mask_;

        /// The start time point of execution.
        SchedTime runtime_;  // Physical time
        SchedTime time_slice_;
        SchedTime start_time_;

        SchedTime deadline_; // Virtual time
        SchedTime vruntime_; // Virtual time
        IScheduler *scheduler_;
        ustl::collections::intrusive::AnyMemberHook<> managed_hook_;
    public:
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedOption);
    };

} // namespace ours::task

#define SCHED_OBJECT_INTERFACE

#endif // OURS_TASK_SCHED_STATE_HPP
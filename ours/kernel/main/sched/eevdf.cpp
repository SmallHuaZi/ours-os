#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>

#include <ustl/ratio.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/collections/intrusive/set.hpp>

using namespace ustl::collections::intrusive;

namespace ours::sched {
    struct EevdffObjectCompare {
        CXX23_STATIC
        auto operator()(SchedObject const &x, SchedObject const &y) -> bool {
            return x.deadline() < y.deadline(); 
        }
    };

    using RunQueueOption = AnyToSetHook<SchedObject::ManagedOption>;
    using RunQueue = MultiSet<SchedObject, RunQueueOption, Compare<EevdffObjectCompare>>;

    class EevdfScheduler: public IScheduler {
        typedef IScheduler      Base;
        typedef EevdfScheduler  Self;
      public:
        static auto durantion_cast(SchedTime duration, SchedWeight weight) -> SchedTime;

        FORCE_INLINE
        auto total_weight() -> SchedWeight {
            if (current_ && current_->on_queue_) {
                return weight_sum_ + current_->weight();
            }

            return weight_sum_;
        }

        OURS_SCHEDULER_API;
      private:
        auto is_eligible(SchedObject const &) const -> bool;

        auto update_current() -> void;

        auto update_min_vruntime() -> void;

        RunQueue runqueue_;

        SchedObject *current_;

        SchedTime min_vruntime_;
        SchedTime weighted_vruntime_sum_;
        SchedWeight weight_sum_;
    };

    /// From physical time duration to virtual.
    FORCE_INLINE
    auto EevdfScheduler::durantion_cast(SchedTime duration, SchedWeight weight) -> SchedTime {
        if (weight == kMinWeight) {
            return duration;
        }

        auto const rate = weight / kMinWeight;
        return duration * rate;
    }

    /// First, `IdealServiceVirtualTime` is same to all task.
    ///
    /// Lag(i) = IdealServiceTime - OwnedServiceTime(i) 
    ///        = Weight(i) * (IdealServiceVirtualTime - OwnedServiceVirtualTime(i))
    ///
    /// Out of \Sum Lag(i) = \Sum Weight(i) * (`IdealServiceVirtualTime` - OwnedServiceVirtualTime(i))
    ///                    = \Sum Weight(i) * (`IdealServiceVirtualTime` - OwnedServiceVirtualTime(i))
    ///                    = TotalWeight * `IdealServiceVirtualTime` - \Sum Weight(i) * OwnedServiceVirtualTime(i)
    ///                    = TotalWeight * `IdealServiceVirtualTime` - WeightedOwnedVirtualTimeSum 
    ///                    = 0
    /// So `IdealServiceVirtualTime` = WeightedOwnedVirtualTimeSum / TotalWeight
    ///                              = WeightedAverageOwnedVirtualTime
    /// 
    /// When the lag of a task is greater than or equal to zero, it is eligible,
    /// otherwise ineligible.
    auto EevdfScheduler::is_eligible(SchedObject const &object) const -> bool {
        auto weighted_vruntime_sum = weighted_vruntime_sum_;
        auto weight_sum = weight_sum_;
        if (current_ && current_->on_queue_) {
            weighted_vruntime_sum += (current_->vruntime() - min_vruntime_) * current_->weight();
            weight_sum += current_->weight();
        }

        return weighted_vruntime_sum >= ((object.vruntime() - min_vruntime_) * weight_sum);
    }

    auto EevdfScheduler::on_tick() -> void {
        update_current();
    }

    /// In order to provide latency guarantees for different request sizes
    /// EEVDF selects the best runnable task from two criteria:
    ///  (1) the task must be eligible (must be owed service)
    ///
    ///  (2) from those tasks that meet (1), we select the one
    ///      with the earliest virtual deadline.
    auto EevdfScheduler::evaluate_next() -> SchedObject * {
        auto iter = runqueue_.begin();
        for (auto const last = runqueue_.end(); iter != last; ++iter) {
            if (!is_eligible(*iter)) {
                break;
            }
        }

        return ustl::mem::address_of(*iter);
    }

    auto EevdfScheduler::dequeue(SchedObject &object) -> void {
        if (!object.on_queue_) {
            return;
        }

        weighted_vruntime_sum_ -= object.vruntime() * object.weight();
        weight_sum_ -= object.weight();

        update_current();
        runqueue_.erase(runqueue_.iterator_to(object));

        num_runnable_ -= 1;
        object.on_queue_ = false;
    }

    auto EevdfScheduler::enqueue(SchedObject &object) -> void {
        if (&object == current_) {
            if (object.vlag().count() != 0) {
                // Rectifies current scheduling object.
                object.vruntime_ -= object.vlag() * (object.weight() + total_weight());
                auto const vslice = durantion_cast(object.time_slice_, object.weight());
                object.deadline_ = object.vruntime_ + vslice;
            }
        }

        weighted_vruntime_sum_ += object.vruntime() * object.weight();
        weight_sum_ += object.weight();

        update_current();
        runqueue_.insert(object);

        num_runnable_ += 1;
        object.on_queue_ = true;
    }

    auto EevdfScheduler::update_current() -> void {
        auto const now = current_time();
        auto const delta = now - common_data_->current_started_time;

        if (delta < SchedTime::zero()) [[unlikely]] {
            return;
        }

        // Update real time.
        common_data_->current_started_time = now;
        current_->runtime_ += delta;

        // Update virtual time.
        auto const scheduling_period = common_data_->scheduling_period();
        auto const rate = current_->weight() / kMinWeight;
        current_->vruntime_ += durantion_cast(scheduling_period, current_->weight());
        if (current_->deadline() <= current_->vruntime()) {
            current_->preemption_state().set_pending();

            // Recalculate current's deadline for next scheduling.
            current_->deadline_ = current_->vruntime_ + current_->time_slice_ * rate;
        }

        update_min_vruntime();
    }

    /// RelativeWeightedOwnedVirtualTime(i) = WeightedOwnedVirtualTime(i) - MinimalOwnedVirtualTime;
    /// RelativeWeightedOwnedVirtualTimeSum = \Sum RelativeWeightedOwnedVirtualTime(i)
    /// WeightedOwnedVirtualTimeSum = RelativeWeightedOwnedVirtualTimeSum + MinimalOwnedVirtualTime * NumTasks
    ///
    /// If MinimalOwnedVirtualTime will be plus Delta, then
    ///     RelativeWeightedOwnedVirtualTime(i) -= Delta
    /// So that
    ///     RelativeWeightedOwnedVirtualTimeSum -= Delta * TotalWeight
    auto EevdfScheduler::update_min_vruntime() -> void {
        auto min_vruntime = min_vruntime_;
        if (current_ && current_->vruntime_ > min_vruntime) {
            min_vruntime = current_->vruntime_;
        }

        // TODO(SmallHuaZi) Remove the constraint that need to search in runqueue
        // for a potential minimal vruntime.

        if (auto delta = min_vruntime - min_vruntime_;  delta > decltype(delta)::zero()) {
            min_vruntime_ = min_vruntime;
            weighted_vruntime_sum_ -= delta * weight_sum_;
        }
    }
}
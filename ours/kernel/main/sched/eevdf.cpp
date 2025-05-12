#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>

#include <ours/task/thread.hpp>

#include <ustl/ratio.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/collections/intrusive/set.hpp>

#include <logz4/log.hpp>

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
            auto current = &common_data_->curr_thread_->sched_object();
            if (current->on_queue_) {
                return weight_sum_ + current->weight();
            }

            return weight_sum_;
        }

        OURS_SCHEDULER_API;
      private:
        auto is_eligible(SchedObject const &) const -> bool;

        auto update_current() -> void;

        auto update_min_vruntime() -> void;

        auto dequeue_object(SchedObject &object) -> void;

        auto enqueue_object(SchedObject &object) -> void;

        RunQueue runqueue_;
        SchedTime min_vruntime_;
        SchedTime weighted_vruntime_sum_;
        SchedWeight weight_sum_;
    };

    CPU_LOCAL
    static EevdfScheduler s_eevdf_scheduler;

    INIT_DATA
    IScheduler *g_eevdf_scheduler = &s_eevdf_scheduler;

    FORCE_INLINE
    static auto trace_thread(SchedObject &object, char const *msg) -> void {
        // Debug code
        {
            auto thread = task::Thread::of(&object);
            log::trace("{} thread name: {}", msg, thread->name());
        }
    }

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
        auto current = &common_data_->curr_thread_->sched_object();
        if (current->on_queue_) {
            weighted_vruntime_sum += (current->vruntime() - min_vruntime_) * current->weight();
            weight_sum += current->weight();
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
            if (is_eligible(*iter)) {
                break;
            }
        }

        if (iter == runqueue_.end()) {
            return 0;
        }

        auto &so = *iter;
        put_prev(common_data_->curr_thread_->sched_object());
        set_next(so);

        return &so;
    }

    auto EevdfScheduler::dequeue_object(SchedObject &object) -> void {
        runqueue_.erase(runqueue_.iterator_to(object));

        weighted_vruntime_sum_ -= object.vruntime() * object.weight();
        weight_sum_ -= object.weight();
        num_runnable_ -= 1;
        object.on_queue_ = false;
    }

    auto EevdfScheduler::dequeue(SchedObject &object) -> void {
        update_current();

        if (!object.on_queue_) {
            return;
        }

        if (&object != &common_data_->curr_thread_->sched_object()) {
            dequeue_object(object);
        }
    }

    auto EevdfScheduler::enqueue_object(SchedObject &object) -> void {
        // Debug code
        {
            auto thread = task::Thread::of(&object);
            log::trace("Thread name: {}", thread->name());
        }

        runqueue_.insert(object);

        weighted_vruntime_sum_ += object.vruntime() * object.weight();
        weight_sum_ += object.weight();
        num_runnable_ += 1;
        object.on_queue_ = true;
    }

    auto EevdfScheduler::enqueue(SchedObject &object) -> void {
        auto current = &common_data_->curr_thread_->sched_object();
        if (&object == current) {
            if (object.vlag().count() != 0) {
                // Rectifies current scheduling object.
                object.vruntime_ -= object.vlag() * (object.weight() + total_weight());
                auto const vslice = durantion_cast(object.time_slice_, object.weight());
                object.deadline_ = object.vruntime_ + vslice;
            }
        }

        update_current();
        enqueue_object(object);
    }

    auto EevdfScheduler::update_current() -> void {
        auto const now = current_time();
        auto const delta = now - common_data_->current_started_time;

        auto current = &common_data_->curr_thread_->sched_object();
        if (delta < SchedTime::zero()) [[unlikely]] {
            return;
        }

        // Update real time.
        common_data_->current_started_time = now;
        current->runtime_ += delta;

        // Update virtual time.
        auto const scheduling_period = common_data_->scheduling_period();
        auto const rate = current->weight() / kMinWeight;
        current->vruntime_ += durantion_cast(scheduling_period, current->weight());
        if (current->deadline() <= current->vruntime()) {
            current->preemption_state().set_pending();

            // Recalculate current's deadline for next scheduling.
            current->deadline_ = current->vruntime_ + current->time_slice_ * rate;
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
        auto current = &common_data_->curr_thread_->sched_object();
        if (current->vruntime_ > min_vruntime) {
            min_vruntime = current->vruntime_;
        }

        // TODO(SmallHuaZi) Remove the constraint that need to search in runqueue
        // for a potential minimal vruntime.

        if (auto delta = min_vruntime - min_vruntime_;  delta > decltype(delta)::zero()) {
            min_vruntime_ = min_vruntime;
            weighted_vruntime_sum_ -= delta * weight_sum_;
        }
    }

    auto EevdfScheduler::put_prev(SchedObject &prev) -> void {
        if (!prev.on_queue_) {
            trace_thread(prev, __func__);
            // `current` may not ownes full requested time.
            update_current();
            enqueue_object(prev);
        }
    }

    auto EevdfScheduler::set_next(SchedObject &next) -> void {
        if (next.on_queue_) {
            trace_thread(next, __func__);
            dequeue_object(next);
        }

        auto current = &common_data_->curr_thread_->sched_object();
        current->start_time_ = current_time();
    }

    auto EevdfScheduler::yield() -> void {}
    auto EevdfScheduler::yield(SchedObject &) -> void {}
}
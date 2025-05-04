#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>

#include <ustl/ratio.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/collections/intrusive/set.hpp>

using namespace ustl::collections::intrusive;

namespace ours::sched {
    using AvgRuntime = ustl::Ratio<usize>;

    struct EevdffObjectCompare {
        CXX23_STATIC
        auto operator()(SchedObject const &x, SchedObject const &y) -> bool {
            return x.deadline() < y.deadline(); 
        }
    };

    using RunQueueOption = AnyToSetHook<SchedObject::ManagedOption>;
    using RunQueue = MultiSet<SchedObject, RunQueueOption, Compare<EevdffObjectCompare>>;

    class EevdfScheduler: public IScheduler {
        typedef EevdfScheduler  Self;
      public:
        OURS_SCHEDULER_API;
    
      private:
        auto is_eligible(SchedObject const &) const -> bool;

        auto update(SchedObject &) -> void;

        usize num_runnable_;
        RunQueue runqueue_;

        SchedObject *current_;

        /// Avage runtime = vt_numerator_ / vi_denominator_ + min_vruntime_
        AvgRuntime avg_vruntime_;
        SchedTime min_vruntime_;

        SchedWeight weight_;
    };

    /// When the lag of a task is greater than or equal to zero, it is eligible,
    /// otherwise ineligible.
    auto EevdfScheduler::is_eligible(SchedObject const &object) const -> bool {
        auto const avg_num = avg_vruntime_.numerator();
        auto const avg_den = avg_vruntime_.denominator();
        return avg_num > ((object.vruntime() - min_vruntime_) * avg_den).count();
    }

    auto EevdfScheduler::on_tick() -> void {
    }

    /// In order to provide latency guarantees for different request sizes
    /// EEVDF selects the best runnable task from two criteria:
    ///  (1) the task must be eligible (must be owed service)
    ///
    ///  (2) from those tasks that meet (1), we select the one
    ///      with the earliest virtual deadline.
    /// 
    /// We can do this in O(log n) time due to an augmented RB-tree. The
    /// tree keeps the entries sorted on deadline, but also functions as a
    /// heap based on the vruntime by keeping:
    /// 
    /// Which allows tree pruning through eligibility.
    auto EevdfScheduler::pick_next() -> SchedObject * {
        auto iter = runqueue_.begin();
        for (auto const last = runqueue_.end(); iter != last; ++iter) {
            if (!is_eligible(*iter)) {
                break;
            }
        }

        return ustl::mem::address_of(*iter);
    }

    auto EevdfScheduler::dequeue(SchedObject &object) -> void {
        runqueue_.insert(object);
    }

    auto EevdfScheduler::enqueue(SchedObject &object) -> void {
    }
}
#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>

#include <ustl/collections/intrusive/set.hpp>

using ustl::collections::intrusive::MultiSet;
using ustl::collections::intrusive::AnyToSetHook;

namespace ours::sched {
    class EevdfScheduler
        : public Scheduler
    {
        typedef EevdfScheduler  Self;

    public:
        OURS_SCHEDULER_API;
    
    private:
        auto is_eligible(SchedObject const &) const -> bool;

    private:
        usize runnable_;
        usize interruptable_;

        using RunQueueOption = AnyToSetHook<SchedObject::ManagedOption>;
        using RunQueue = MultiSet<SchedObject, RunQueueOption>;
        RunQueue  task_queue_;
    };

    /// Entity is eligible once it received less service than it ought to have,
    /// eg. lag >= 0.
    ///
    /// lag_i = S - s_i = w_i*(V - v_i)
    ///
    /// lag_i >= 0 -> V >= v_i
    ///
    ///     \Sum (v_i - v)*w_i
    /// V = ------------------ + v
    ///          \Sum w_i
    ///
    /// lag_i >= 0 -> \Sum (v_i - v)*w_i >= (v_i - v)*(\Sum w_i)
    ///
    /// Note: using 'avg_vruntime() > se->vruntime' is inaccurate due
    ///       to the loss in precision caused by the division.
    auto EevdfScheduler::is_eligible(SchedObject const &) const -> bool
    {}

    /// [Earliest Eligible Virtual Deadline First] EEVDF
    /// 
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
    auto EevdfScheduler::pick_next() -> SchedObject *
    {
        auto first = task_queue_.begin();
        for (auto const last = task_queue_.end(); first != last; ++first) {
            if (this->is_eligible(*first)) {
                break;
            }
        }

        return first.operator->();
    }

    auto EevdfScheduler::dequeue(SchedObject &) -> void
    {}

    auto EevdfScheduler::enqueue(SchedObject &) -> void
    {}
}
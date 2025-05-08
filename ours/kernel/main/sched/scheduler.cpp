#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>

#include <ours/task/thread.hpp>

namespace ours::sched {
    auto MainScheduler::evaluate_next_thread(task::Thread *curr, SchedTime now) -> task::Thread * {
        if (auto eevdf = schedulers_[kEevdf];
            curr->sched_object().get_scheduler() == eevdf && num_runnable_ == eevdf->num_runnable_) {
            if (auto task = eevdf->evaluate_next()) {
                return task::Thread::from_sched_object(task);
            }
        }

        for (auto scheduler : schedulers_) {
            if (auto task = scheduler->evaluate_next()) {
                return task::Thread::from_sched_object(task);
            }
        }

        // There must be at least one thread, such as the idle thread.
        unreachable();
    }

    auto MainScheduler::reschedule_common(task::Thread *curr, SchedTime now) -> void {
        auto next = evaluate_next_thread(curr, now);
        auto const thread_changed = next != curr;

        auto &curr_so = curr->sched_object();
        // Everything be ok. Let us do the final and actual switch to context.
        if (thread_changed) [[likely]] {
            num_switches_ += 1;

            // Make sure that the same aspace do not be changed, because that 
            // lead to flush TLB fully without global pages.
            if (curr->aspace() != next->aspace()) {
                mem::VmAspace::switch_aspace(curr->aspace(), next->aspace());
            }
            curr_so.preemption_state().clear_pending();

            set_prev_thread(curr);

            task::Thread::switch_context(curr, next);
        }
    }

    auto MainScheduler::reschedule() -> void {

    }

    auto MainScheduler::activate(task::Thread &thread) -> void {
        
    }

    auto MainScheduler::deactivate(task::Thread &thread) -> void {
        
    }

    auto MainScheduler::init_thread(task::Thread &thread, BaseProfile const &profile) -> void {
        auto &so = thread.sched_object();
        new (&so) SchedObject(profile);

        switch (profile.discipline) {
            case SchedDiscipline::Eevdf:
                // Bind with scheduler.
                so.set_scheduler(schedulers_[kEevdf]);
                break;
            default:
                panic("No other disciplines");
        }
    }

    auto MainScheduler::enqueue_thread(task::Thread &thread, SchedTime now) -> void {
        auto &so = thread.sched_object();
        so.get_scheduler()->enqueue(so);
    }

    auto MainScheduler::dequeue_thread(task::Thread &thread) -> void {
        auto &so = thread.sched_object();
        so.get_scheduler()->dequeue(so);
    }

    auto MainScheduler::tick() -> void {
        task::Thread::Current::sched_object().get_scheduler()->on_tick();
    }

    auto MainScheduler::update_timeline(SchedTime now) -> void {
    }

} // namespace ours::sched
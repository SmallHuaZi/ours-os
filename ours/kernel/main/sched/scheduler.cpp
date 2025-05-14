#include <ours/sched/scheduler.hpp>
#include <ours/sched/sched_object.hpp>
#include <ours/cpu-states.hpp>
#include <ours/task/thread.hpp>
#include <ours/task/timer-queue.hpp>
#include <ours/arch/mp.hpp>

namespace ours::sched {
    auto MainScheduler::block(task::Thread &thread) -> void {
        DEBUG_ASSERT(&thread == task::Thread::Current::get(), "Block a un-running thread");
        MainScheduler::reschedule(thread);
    }

    auto MainScheduler::assign_target_cpu(task::Thread &thread) -> CpuNum {
        auto &so = thread.sched_object();
        auto last_cpu = so.recent_cpu_;
        auto curr_cpu = CpuLocal::cpunum();
        auto starting_cpu = last_cpu == kInvalidCpuNum ? curr_cpu : last_cpu;

        auto const active_mask = peek_active_mask();
        auto const available_mask = so.get_available_mask(active_mask);

        auto target_cpu = kInvalidCpuNum;
        for_each_online_cpu([available_mask, &target_cpu] (CpuNum cpunum) {
            if (!available_mask.test(cpunum)) {
                // It is not a eligible candidate CPU.
                return Status::ShouldRetry;
            }

            // Find mininal expected runtime CPU
            if (target_cpu == kInvalidCpuNum ||
                Self::get(target_cpu)->common_data_.total_expected_runtime_ > 
                Self::get(cpunum)->common_data_.total_expected_runtime_) {
                target_cpu = cpunum;
            }

            return Status::Ok;
        });

        DEBUG_ASSERT(target_cpu != kInvalidCpuNum);
        return target_cpu;
    }

    auto MainScheduler::unblock(task::Thread &thread) -> void {
        CpuNum target_cpu = kInvalidCpuNum;
        auto &so = thread.sched_object();
        while (target_cpu == kInvalidCpuNum) {
            target_cpu = assign_target_cpu(thread);
            auto const recent_cpu = thread.recent_cpu();
            auto const need_migration = recent_cpu != kInvalidCpuNum && 
                                        recent_cpu != target_cpu;

            if (need_migration) {
                target_cpu = recent_cpu;
            }

            auto const scheduler = Self::get(target_cpu);
            if (need_migration) {
            } else {
                so.current_cpu_ = target_cpu;
                if (!so.on_queue_) {
                    scheduler->enqueue_thread(thread, current_time());
                }
            }
        }

        if (target_cpu == CpuLocal::cpunum()) {
            auto current = task::Thread::Current::get();
            // On local CPU, directly executes preemption.
            if (current->sched_object().preemption_state().is_preemptible()) {
                preempt();
            }
        } else {
            // On remote CPU, send a rescheduling IPI to it.
            mp_reschedule(CpuMask::from_cpu_num(target_cpu), 0);
        }
    }

    auto MainScheduler::preempt() -> void {
        auto current = task::Thread::Current::get();
        get()->reschedule_common(current, current_time());
    }

    auto MainScheduler::evaluate_next_thread(task::Thread *curr, SchedTime now) -> task::Thread * {
        if (auto eevdf = schedulers_[kEevdf];
            curr->sched_object().get_scheduler() == eevdf && num_runnable_ == eevdf->num_runnable_) {
            if (auto so = eevdf->evaluate_next(curr->sched_object())) {
                return task::Thread::of(so);
            }
        }

        for (auto scheduler : schedulers_) {
            if (auto so = scheduler->evaluate_next(curr->sched_object())) {
                return task::Thread::of(so);
            }
        }

        // No any thread exists, so return `current` as next thread.
        return curr;
    }

    auto MainScheduler::put_prev_thread(task::Thread *prev) -> void {
        DEBUG_ASSERT(prev);
        auto &so = prev->sched_object();
        so.get_scheduler()->put_prev(so);
        common_data_.prev_thread_ = prev;
    }

    auto MainScheduler::set_next_thread(task::Thread *next) -> void {
        DEBUG_ASSERT(next);
        auto &so = next->sched_object();
        so.get_scheduler()->set_next(so);
        common_data_.curr_thread_ = next;
    }

    auto MainScheduler::reschedule_common(task::Thread *curr, SchedTime now) -> void {
        if (curr->state() == task::ThreadState::Running) {
            curr->set_ready();
        }

        auto next = evaluate_next_thread(curr, now);
        auto const thread_changed = next != curr;

        auto &curr_so = curr->sched_object();

        // Everything be ok. Let us do the final and actual switch to context.
        if (thread_changed) [[likely]] {
            num_switches_ += 1;

            curr_so.preemption_state().clear_pending();
            next->set_running();

            set_current_thread(next);

            // Make sure that the same aspace do not be changed, because that 
            // lead to flush TLB fully without global pages.
            if (curr->aspace() != next->aspace()) {
                mem::VmAspace::switch_aspace(curr->aspace(), next->aspace());
            }

            task::Thread::switch_context(curr, next);
        }
    }

    auto MainScheduler::reschedule(task::Thread &thread) -> void {
        get()->reschedule_common(&thread, current_time());
    }

    auto MainScheduler::init_thread(task::Thread &thread, BaseProfile const &profile) -> void {
        canary_.verify();

        auto &so = thread.sched_object();
        new (&so) SchedObject(profile);

        switch (profile.discipline) {
            case SchedDiscipline::Eevdf:
                // Bind with scheduler.
                so.set_scheduler(schedulers_[kEevdf]);
                so.affinity_mask_ = global_cpu_states().online_cpus;
                so.time_slice_ = SchedCommonData::kDefaultMinSchedGranularity;
                so.on_queue_ = false;
                break;
            default:
                panic("No other disciplines");
        }

        so.time_slice_ = common_data_.minimal_granularity;
    }

    auto MainScheduler::enqueue_thread(task::Thread &thread, SchedTime now) -> void {
        canary_.verify();
        ustl::sync::LockGuard guard(mutex_);

        update_timeline(current_time());
        auto &so = thread.sched_object();
        so.get_scheduler()->enqueue(so);

        common_data_.weight_sum_ += so.weight();
        num_runnable_ += 1;
    }

    auto MainScheduler::dequeue_thread(task::Thread &thread) -> void {
        canary_.verify();
        ustl::sync::LockGuard guard(mutex_);

        update_timeline(current_time());
        auto &so = thread.sched_object();
        so.get_scheduler()->dequeue(so);

        common_data_.weight_sum_ -= so.weight();
        num_runnable_ -= 1;
    }

    auto MainScheduler::on_tick() -> void {
        canary_.verify();

        ustl::sync::LockGuard guard(mutex_);
        update_timeline(current_time());
        task::Thread::Current::sched_object().get_scheduler()->on_tick();
    }

    auto MainScheduler::update_timeline(SchedTime now) -> void {
        common_data_.last_updated_time_ = now;
        common_data_.timeline = now;
    }

    extern IScheduler *g_eevdf_scheduler;

    auto MainScheduler::init() -> void {
        new (this) MainScheduler();

        auto raw_schedulers = new (mem::kGafKernel) IScheduler *[1]();
        DEBUG_ASSERT(raw_schedulers);
        auto eevdf = CpuLocal::access(g_eevdf_scheduler);
        DEBUG_ASSERT(eevdf);

        raw_schedulers[0] = eevdf;

        schedulers_ = ustl::views::make_span(raw_schedulers, 1);
        this_cpu_ = CpuLocal::cpunum();

        for (auto scheduler: schedulers_) {
            scheduler->common_data_ = &common_data_;
        }

        auto shadow = s_active_schedulers.load();
        shadow.set(this_cpu_, true);
        s_active_schedulers.store(shadow);

        // task::TimerQueue::current()->reset_preemption_timer(
        //     TimePoint(task::Thread::Current::sched_object().deadline())
        // );
    }

} // namespace ours::sched
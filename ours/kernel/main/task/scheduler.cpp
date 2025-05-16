#include <ours/task/scheduler.hpp>
#include <ours/task/sched_object.hpp>
#include <ours/cpu-states.hpp>
#include <ours/task/thread.hpp>
#include <ours/task/timer-queue.hpp>
#include <ours/arch/mp.hpp>

#include <arch/intr_disable_guard.hpp>

namespace ours::task {
    extern IScheduler *g_fair_scheduler;

    auto MainScheduler::enqueue_thread(Thread *thread, SchedTime now) -> void {
        canary_.verify();
        ustl::sync::LockGuard guard(mutex_);

        update_timeline(now);
        auto &so = thread->sched_object();
        so.get_scheduler()->enqueue_thread(thread);

        common_data_.weight_sum_ += so.weight();
        num_runnable_ += 1;
    }

    auto MainScheduler::dequeue_thread(Thread *thread, SchedTime now) -> void {
        canary_.verify();
        ustl::sync::LockGuard guard(mutex_);

        update_timeline(now);
        auto &so = thread->sched_object();
        so.get_scheduler()->dequeue_thread(thread);

        common_data_.weight_sum_ -= so.weight();
        num_runnable_ -= 1;
    }

    auto MainScheduler::put_prev_and_set_next_thread(Thread *curr, 
                                                     Thread *next) -> void {
        if (next == curr) {
            return;
        }

        auto &curr_so = curr->sched_object();
        curr_so.get_scheduler()->put_prev_thread(curr);
        curr->set_ready();

        auto &next_so = next->sched_object();
        next_so.get_scheduler()->set_next_thread(next);
        next->set_running();

        common_data_.prev_thread_ = curr;
        Thread::Current::set(next);
    }

    auto MainScheduler::pick_next_thread(Thread *curr, SchedTime now) -> Thread * {
        for (auto scheduler : schedulers_) {
            if (auto next = scheduler->pick_next_thread(curr)) {
                put_prev_and_set_next_thread(curr, next);
                return next;
            }
        }

        if (auto next = steal_work()) {
            put_prev_and_set_next_thread(curr, next);
            return next;
        }

        auto const state = curr->state();
        if (state != ThreadState::Terminated && 
            state != ThreadState::Blocking &&
            state != ThreadState::Sleeping) {
            return curr;
        }

        // No any thread active, so return `current` as next thread.
        return idler_;
    }

    auto MainScheduler::switch_context(Thread *curr, Thread *next) -> void {
        // Make sure that the same aspace do not be changed, because that 
        // lead to flush TLB fully without global pages.
        //
        // Note at here we assump that all user address spaces include a duplication
        // of kernel address space.
        if (curr->aspace() != next->aspace()) {
            mem::VmAspace::switch_aspace(curr->aspace(), next->aspace());
        }

        Thread::switch_context(curr, next);
    }

    auto MainScheduler::reschedule(Thread *curr, SchedTime now) -> void {
        update_timeline(now);

        arch::IntrDisableGuard guard{};
        ustl::sync::LockGuard this_guard{mutex_};

        auto next = pick_next_thread(curr, now);
        DEBUG_ASSERT(next, "There are at least one thread");

        // Everything be ok. Let us do the final and actual switch to context.
        if (next != curr) [[likely]] {
            num_switches_ += 1;

            curr->sched_object().preemption_state().clear_pending();
            switch_context(curr, next);
        }
    }

    auto MainScheduler::deactivate_thread(Thread *thread) -> void {
        DEBUG_ASSERT(thread == Thread::Current::get(), "Block a un-running thread");
        reschedule(thread, current_time());
    }

    auto MainScheduler::assign_target_cpu(Thread *thread) -> CpuNum {
        DEBUG_ASSERT(thread);

        auto &so = thread->sched_object();
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
                Current::get(target_cpu)->common_data_.total_expected_runtime_ > 
                Current::get(cpunum)->common_data_.total_expected_runtime_) {
                target_cpu = cpunum;
            }

            return Status::Ok;
        });

        DEBUG_ASSERT(target_cpu != kInvalidCpuNum);
        return target_cpu;
    }

    auto MainScheduler::activate_thread(Thread *thread) -> void {
        CpuNum target_cpu = kInvalidCpuNum;
        auto &so = thread->sched_object();
        while (target_cpu == kInvalidCpuNum) {
            target_cpu = assign_target_cpu(thread);
            auto const recent_cpu = thread->recent_cpu();
            auto const need_migration = recent_cpu != kInvalidCpuNum && 
                                        recent_cpu != target_cpu;

            // The recent CPU the thread ran on is not the CPU assigned, so we need to
            // migrate it to other CPU.
            if (need_migration) {
                target_cpu = recent_cpu;
            }

            auto const scheduler = Current::get(target_cpu);
            ustl::sync::LockGuard guard(scheduler->mutex_);

            // If the scheduler running on |target_cpu| is not active, skip it and
            // and then find next CPU.
            if (!scheduler->is_active()) {
                continue;
            }

            if (need_migration) {
                // Now we do not handle the migration case.
            } else {
                so.current_cpu_ = target_cpu;
                if (!so.on_queue_) {
                    scheduler->enqueue_thread(thread, current_time());
                }
            }
        }

        thread->mutex().unlock();

        if (target_cpu == CpuLocal::cpunum()) {
            auto current = Thread::Current::get();
            // On local CPU, directly executes preemption.
            if (current->sched_object().preemption_state().is_preemptible()) {
                preempt();
            }
        } else {
            // On remote CPU, send a rescheduling IPI to it.
            mp_reschedule(CpuMask::from_cpu_num(target_cpu), 0);
        }
    }

    auto MainScheduler::reset_preemption(CpuNum cpu, TimePoint now, TimePoint deadline) -> void {
        task::TimerQueue::get(cpu)->reset_preemption_timer(deadline);
    }

    auto MainScheduler::init_thread(Thread &thread, BaseProfile const &profile) -> void {
        canary_.verify();

        auto &so = thread.sched_object();
        new (&so) SchedObject(profile);

        switch (profile.discipline) {
            case SchedAlgorithm::Eevdf:
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

    auto MainScheduler::on_tick() -> void {
        canary_.verify();

        ustl::sync::LockGuard guard(mutex_);
        update_timeline(current_time());
        Thread::Current::sched_object().get_scheduler()->on_tick();
    }

    auto MainScheduler::update_timeline(SchedTime now) -> void {
        common_data_.last_updated_time_ = now;
        common_data_.timeline = now;
    }

    auto MainScheduler::init() -> void {
        new (this) MainScheduler();

        extern IScheduler *g_eevdf_scheduler;

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
        //     TimePoint(Thread::Current::sched_object().deadline())
        // );
    }

} // namespace ours::task
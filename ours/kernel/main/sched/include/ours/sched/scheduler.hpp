#ifndef OURS_SCHED_SCHEDULER_HPP
#define OURS_SCHED_SCHEDULER_HPP 1

#include <ours/sched/cfg.hpp>
#include <ours/sched/sched_object.hpp>

#include <ustl/chrono/duration.hpp>

namespace ours::sched {
    class SchedObject;
    struct SchedCommonData;

    class Scheduler
    {
        typedef Scheduler   Self;

    public:
        virtual auto init() -> void = 0;

        virtual auto enqueue(SchedObject &obj) -> void = 0;

        virtual auto dequeue(SchedObject &obj) -> void = 0;

        virtual auto yield() -> void = 0;

        virtual auto yield(SchedObject &obj) -> void = 0;

        virtual auto pick_next() -> SchedObject * = 0;

        virtual auto set_next(SchedObject &obj) -> void = 0;

        OURS_CFG_SCHED(PREEMPT, 
            auto preempt(SchedObject &obj) -> void
        );

        OURS_CFG_SCHED(MIGRATE,
            auto migrate(SchedObject &obj) -> void
        );
    
    private:
        SchedCommonData *common_data_;
    };

} // namespace ours

#define OURS_SCHEDULER_API \
    virtual auto pick_next() -> SchedObject * override; \
    virtual auto enqueue(SchedObject &obj) -> void override; \
    virtual auto dequeue(SchedObject &obj) -> void override;

#endif // #ifndef OURS_SCHED_SCHEDULER_HPP
#include <ours/sched/mod.hpp>
#include <ours/sched/sched_object.hpp>

namespace ours::sched {
    template <>
    auto reschedule(SchedObject *object) -> void
    {}

} // namespace ours::sched
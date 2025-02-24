#ifndef OURS_SCHED_MOD_HPP
#define OURS_SCHED_MOD_HPP 1

namespace ours::sched {
    class SchedObject;

    /// \p object
    template <typename Schedulable>
    auto reschedule(Schedulable *schedulable) -> void
    {  return reschedule(static_cast<SchedObject *>(schedulable));  }

} // namespace ours::sched

#endif // #ifndef OURS_SCHED_MOD_HPP
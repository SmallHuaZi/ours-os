#ifndef OURS_SCHED_TYPES_HPP
#define OURS_SCHED_TYPES_HPP 1

#include <ustl/function/function.hpp>

namespace ours::task {
    using ThreadStartEntry = ustl::function::Fn<auto () -> void>;

} // namespace ours::task

#endif // #ifndef OURS_SCHED_TYPES_HPP
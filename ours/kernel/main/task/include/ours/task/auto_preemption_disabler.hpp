/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef OURS_TASK_AUTO_PREEMPTION_DISABLER_HPP
#define OURS_TASK_AUTO_PREEMPTION_DISABLER_HPP 1

#include <ours/task/thread.hpp>

namespace ours::task {
    class AutoPreemptionDisabler {
      public:
        FORCE_INLINE
        AutoPreemptionDisabler() {
            task::Thread::Current::preemption_state().disable_preemption();
            disabled_ = true;
        }

        FORCE_INLINE
        auto reenable() -> void {
            if (!disabled_) {
                return;
            }

            disabled_ = false;
            task::Thread::Current::preemption_state().enable_preemption();
        }

        FORCE_INLINE
        ~AutoPreemptionDisabler() {
            reenable();
        }

      private:
        bool disabled_ = false;
    };

} // namespace ours::task

#endif // #ifndef OURS_TASK_AUTO_PREEMPTION_DISABLER_HPP
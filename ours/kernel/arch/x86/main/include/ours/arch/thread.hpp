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
#ifndef OURS_ARCH_THREAD_HPP
#define OURS_ARCH_THREAD_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>
#include <ours/cpu-local.hpp>

#include <ours/task/types.hpp>

namespace ours::task {
    class X86Thread {
        typedef X86Thread  Self;
      public:
        static auto switch_context(Self *prev, Self *next) -> void;

        FORCE_INLINE
        static auto current_thread() -> Self * {
            return CpuLocal::read(s_current_arch_thread);
        }

      private:
        FORCE_INLINE
        static auto set_current_thread(Self *curr) -> void {
            return CpuLocal::write(s_current_arch_thread, curr);
        }

        friend class Thread;
        VirtAddr fs_base_;
        VirtAddr gs_base_;
        VirtAddr sp_;

        static Self *s_current_arch_thread;
    };

    typedef X86Thread   ArchThread;

} // namespace ours::task

#endif // #ifndef OURS_ARCH_THREAD_HPP
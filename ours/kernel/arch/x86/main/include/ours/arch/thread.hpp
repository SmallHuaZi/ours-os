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

namespace ours::task {
    class X86Thread {
        typedef X86Thread  Self;
      public:
        static auto switch_context(Self &prev, Self &next) -> void;
    
      private:
        friend class Thread;
        VirtAddr fs_base_;
        VirtAddr gs_base_;
    };

    typedef X86Thread   ArchThread;

} // namespace ours::task

#endif // #ifndef OURS_ARCH_THREAD_HPP
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
#ifndef OURS_MEM_STACK_HPP
#define OURS_MEM_STACK_HPP 1

#include <ours/mem/types.hpp>
#include <ours/mem/vm_mapping.hpp>

#include <ustl/util/noncopyable.hpp>

namespace ours::mem {
    class Stack {
        typedef Stack   Self;
        USTL_NO_MOVEABLE_AND_COPYABLE(Stack);
      public:
        CXX11_CONSTEXPR
        static auto const kDefaultStackSize = KB(16); // Two pages.

        Stack() = default;

        auto init(usize size = kDefaultStackSize) -> Status;

        auto init_first() -> void;

        FORCE_INLINE
        auto top() const -> VirtAddr {
            return stack_->base() + stack_->size();
        }

      private:
        ustl::Rc<VmMapping> stack_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_STACK_HPP
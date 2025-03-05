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

#ifndef OURS_OBJECT_KERNEL_OBJECT_HPP
#define OURS_OBJECT_KERNEL_OBJECT_HPP 1

#include <ours/types.hpp>

#include <ustl/rc.hpp>
#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>

namespace ours::object {
    class KernelObject
        : private ustl::RefCounter<KernelObject>
    {
        typedef KernelObject   Self;
    public:
        auto id() const -> KoId
        {  return koid_;  }

        auto name() const -> char const *
        {  return this->name_;  }

        auto set_name(char const *name) -> void
        {}

        auto add_observer()
        {}

        auto remove_observer()
        {}

    protected:
        KernelObject(char const *name);
        KernelObject(KernelObject const &other);

    protected:
        KoId koid_;
        ustl::sync::AtomicU32  handle_count_;
        ustl::sync::AtomicU32  signal_count_;

        static constexpr u32 const MAX_NAME_SIZE = 32;
        char name_[MAX_NAME_SIZE];
    };

} // namespace ours

#endif // #ifndef OURS_OBJECT_KERNEL_OBJECT_HPP

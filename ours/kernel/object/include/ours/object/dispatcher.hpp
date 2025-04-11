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
#ifndef OURS_OBJECT_DISPATCHER_HPP
#define OURS_OBJECT_DISPATCHER_HPP 1

#include <ours/types.hpp>
#include <ours/rights.hpp>
#include <ours/config.hpp>

#include <ustl/rc.hpp>
#include <ustl/option.hpp>
#include <ustl/sync/atomic.hpp>

namespace ours::object {
    class Dispatcher
        : private ustl::RefCounter<Dispatcher>
    {
        typedef Dispatcher   Self;
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
        Dispatcher(char const *name);
    protected:
        KoId koid_;
        ustl::sync::AtomicU32  handle_count_;
        ustl::sync::AtomicU32  signal_count_;

        CXX11_CONSTEXPR
        static auto const MAX_NAME_SIZE = 32;
        char name_[MAX_NAME_SIZE];
    };

    template <typename Derived, Rights default_rights, Signals>
    class SoloDispatcher
        : public Dispatcher
    {

    };

} // namespace ours

#endif // #ifndef OURS_OBJECT_DISPATCHER_HPP

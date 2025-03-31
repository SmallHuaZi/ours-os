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
#ifndef OURS_OBJECT_HANDLE_HPP
#define OURS_OBJECT_HANDLE_HPP 1

#include <ustl/rc.hpp>

namespace ours::object {
    class Dispatcher;

    template <typename KernelObject>
    struct KernelHandle {
        static_assert(ustl::traits::IsBaseOfV<Dispatcher, KernelObject>, "");
        typedef KernelObject   Object;

    private:
        ustl::Rc<Object> dispatcher_;
    };

} // namespace ours::object

#endif // #ifndef OURS_OBJECT_HANDLE_HPP
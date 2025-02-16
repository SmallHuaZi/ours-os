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

#ifndef KMRD_DAMON_HPP
#define KMRD_DAMON_HPP 1

#include <ours/task/thread.hpp>

namespace kmrd {
    class Damon
    {

        ours::task::Thread  *thread_;
    };

} // namespace kmrd

#endif // #ifndef KMRD_DAMON_HPP
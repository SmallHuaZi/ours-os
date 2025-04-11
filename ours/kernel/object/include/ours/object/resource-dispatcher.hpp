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
#ifndef OURS_OBJECT_RESOURCE_DISPATCHER_HPP
#define OURS_OBJECT_RESOURCE_DISPATCHER_HPP

#include <ours/object/dispatcher.hpp>

namespace ours::object {
    class ResourceDispatcher;
    using ResourceDispatcherBase = SoloDispatcher<ResourceDispatcher, Rights::Read, {}>;

    class ResourceDispatcher: public ResourceDispatcherBase {
    
    private:
        PhysAddr base_;
        PhysAddr size_;
    };
} // namespace ours::object

#endif // OURS_OBJECT_RESOURCE_DISPATCHER_HPP

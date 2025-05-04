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
#define OURS_OBJECT_RESOURCE_DISPATCHER_HPP 1

#include <ours/object/dispatcher.hpp>
#include <ktl/name.hpp>

namespace ours::object {
    class ResourceDispatcher;
    using ResourceDispatcherBase = SoloDispatcher<ResourceDispatcher, Rights::Read, {}>;

    class ResourceDispatcher: public ResourceDispatcherBase {
      public:
        CXX11_CONSTEXPR
        static auto const kMaxNameLength = 32;
        
      private:
        PhysAddr base_;
        PhysAddr size_;
        ktl::Name<kMaxNameLength> name_;
    };
} // namespace ours::object

#endif // OURS_OBJECT_RESOURCE_DISPATCHER_HPP

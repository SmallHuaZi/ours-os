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
#include <ours/object/handle.hpp>
#include <ours/syscall/resource.hpp>

#include <ustl/collections/intrusive/list.hpp>

namespace ours::object {
    class ResourceDispatcher;
    using ResourceDispatcherBase = SoloDispatcher<ResourceDispatcher, kDefaultRights>;

    class ResourceDispatcher: public ResourceDispatcherBase,
                              public ustl::collections::intrusive::ListBaseHook<> {
        typedef ResourceDispatcher      Self;
        typedef ResourceDispatcherBase  Base;
      public:
        CXX11_CONSTEXPR
        static auto const kMaxNameSize = 32;

        typedef ktl::Name<kMaxNameSize> Name;
        typedef ustl::collections::intrusive::List<Self>    ResourceList;

        struct RsrcStorage {
            ResourceList resources_;
        };

        static auto create(KernelHandle<Self> *out, RsrcKind kind, PhysAddr base, PhysAddr size, 
                           RsrcFlags flags, Name const &name) -> Status;
      private:
        ResourceDispatcher(RsrcKind kind, PhysAddr base, PhysAddr size, RsrcFlags flags, Name const &);

        GKTL_CANARY(ResourceDispatcher, canary_);
        Name name_;
        RsrcKind kind_;
        PhysAddr base_;
        PhysAddr size_;
        RsrcFlags flags_;
        ResourceList *resource_list_;

        static inline RsrcStorage s_system_resource_;
    };
} // namespace ours::object

#endif // OURS_OBJECT_RESOURCE_DISPATCHER_HPP

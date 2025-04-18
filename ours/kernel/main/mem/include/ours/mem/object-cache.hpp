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
#ifndef OURS_MEM_OBJECT_CACHE_HPP
#define OURS_MEM_OBJECT_CACHE_HPP 1

#include <ours/mem/gaf.hpp>
#include <ours/mem/scope.hpp>
#include <ours/mem/pm_frame.hpp>

#include <ours/mutex.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/ref_counter.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/collections/intrusive/slist.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    class ObjectCache
        : public ustl::RefCounter<ObjectCache>
    {
        typedef ObjectCache     Self;

        template <typename T, typename... Args> 
        auto allocate(Gaf gaf, NodeId nid, Args &&...args) -> T * {
            DEBUG_ASSERT(sizeof(T) <= object_size_, "The size of object is too large");
            auto const object = allocate(gaf, nid);
            if (!object) {
                return nullptr;
            }
            return ustl::mem::construct_at(static_cast<T *>(object), ustl::forward<Args>(args)...);
        }

        template <typename T> 
        auto deallocate(T *object) -> void {
            DEBUG_ASSERT(object, "Pass a invalid object");
            DEBUG_ASSERT(sizeof(T) <= object_size_, "The size of object is too large");
            ustl::mem::destroy_at(object);
            deallocate(object);
        }
    private:
        using Slab = SlabFrame;
        using SlabList = SlabFrameList<>;
        using Object = Slab::Object;
        using ObjectList = Slab::ObjectList;

        auto allocate(Gaf gaf, NodeId nid) -> Object *;

        auto deallocate(Object *object) -> void;

        auto get_neighbor(Object *) -> Object *;

        auto get_slab(NodeId nid) -> Slab *;
        auto get_slab_from_node(NodeId nid) -> Slab *;
        auto get_slab_any_node() -> Slab *;

        auto alloc_slab(NodeId nid) -> Slab *;

        auto free_slab(Slab *) -> void;

        struct CacheOnCpu;
        struct CacheOnNode;
        GKTL_CANARY(MemCache, canary_);
	    Gaf gaf_;
        u16 order_;
        u16 objects_;
	    u32 object_size_;
	    u32 object_align_;
	    u32 inuse_;
	    const char *name_;
	    PerCpu<CacheOnCpu> cache_cpu_;
        ustl::Array<CacheOnNode *, MAX_NODE> cache_node_;

        ustl::collections::intrusive::ListMemberHook<> managed_hook;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook, ManagedHook);
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_OBJECT_CACHE_HPP
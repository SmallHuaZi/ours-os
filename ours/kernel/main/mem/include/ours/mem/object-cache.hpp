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
#include <ours/mem/cfg.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/pmm.hpp>

#include <ours/mutex.hpp>
#include <ours/cpu-local.hpp>

#include <ustl/rc.hpp>
#include <ustl/mem/object.hpp>
#include <ustl/collections/intrusive/slist.hpp>
#include <ustl/collections/intrusive/list.hpp>

#include <gktl/canary.hpp>

namespace ours::mem {
    namespace uci = ustl::collections::intrusive;

    enum class OcFlags {
        Folio,
        Dma,
        Dma32,
    };
    USTL_ENABLE_ENUM_BITMASK(OcFlags);

    /// When a frame is being used in `ObjectCache`, it's descriptor has the following layout.
    struct Slab: public PageFrameBase {
        typedef Slab   Self;

        /// Dummy object that provides a way convenient to build the list organizing free objects.
        struct Object: public uci::SlistBaseHook<uci::LinkMode<uci::LinkModeType::AutoUnlink>> {};
        using ObjectList = uci::Slist<Object, uci::ConstantTimeSize<false>>;

        static auto create(ObjectCache *oc, Gaf gaf, usize order, usize obi_size, NodeId nid = MAX_NODE) -> Self *;

        auto init(ObjectCache *oc, usize order, usize obi_size) -> Status;

        FORCE_INLINE
        auto destory() -> void {
            DEBUG_ASSERT(num_inuse == 0);
            unlink();
            mem::free_frame(to_pmm(), order());
        }

        auto get_object() -> Object *;

        template <typename T>
        auto get_object() -> T * {
            return reinterpret_cast<T *>(get_object());
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto put_object(Object *object) -> void {
            free_list.push_front(*object);
            num_inuse -= 1;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto has_object() const -> bool {
            return num_inuse != num_objects;
        }

        /// Private on logic.
        ustl::sync::AtomicU16 num_inuse;
        ustl::sync::AtomicU16 num_objects;
        ObjectList free_list;
        ObjectCache *object_cache;
    };
    USTL_DECLARE_LIST_TEMPLATE(Slab, SlabList, uci::ConstantTimeSize<false>);
    static_assert(sizeof(Slab) <= kFrameDescSize, "");

    template <>
    struct RoleViewDispatcher<PfRole::Slab> {
        typedef Slab   Type;
    };

    struct ObjectCachePerNode {
        typedef ObjectCachePerNode  Self;

        static auto create(NodeId nid) -> Self *;

        auto destory() -> void;

        FORCE_INLINE
        auto get_slab() -> Slab * {
            if (!has_slab()) {
                return nullptr;
            }
            return &slabs_partial.front(); 
        }

        FORCE_INLINE
        auto add_slab(Slab *slab) -> void {
            slabs_partial.push_back(*slab);
            num_slabs += 1;
            num_partial += 1;
        }

        FORCE_INLINE
        auto put_slab(Slab *slab) -> void {
            DEBUG_ASSERT(!slab->has_object(), "Attempt to put a partial slab object");
            num_partial -= 1;
            slabs_full.push_back(*slab);
        }

        FORCE_INLINE
        auto has_slab() const -> bool {
            return num_partial != 0;
        }

        /// Slabs which contains partial available objects.
        SlabList<> slabs_partial;
        /// Slabs which contains all objects inused.
        SlabList<> slabs_full;
        Mutex mutex_;
        usize num_slabs;
        usize num_partial;
    };

    class ObjectCache {
        typedef ObjectCache         Self;
        typedef Slab::Object        Object;
        typedef Slab::ObjectList    ObjectList;
    public:
        struct CacheOnCpu;
        struct CacheOnNode;

        static auto create(char const *name, usize obj_size, AlignVal align, OcFlags flags) -> Self *;

        template <typename Object>
        FORCE_INLINE
        static auto create(char const *name, OcFlags flags) -> Self * {
            return create(name, sizeof(Object), alignof(Object), flags);
        }

        INIT_CODE
        static auto create_boot(Self &self, char const *name, usize obj_size, AlignVal align, OcFlags flags) -> Status;

        auto do_allocate(Gaf gaf, NodeId nid) -> Object *;

        FORCE_INLINE
        auto do_allocate(NodeId nid) -> Object * {
            return do_allocate(gaf_, nid);
        }

        FORCE_INLINE
        auto allocate(Gaf gaf, NodeId nid) -> void * {
            return allocate(1, gaf, nid);
        }

        auto allocate(usize n, Gaf gaf, NodeId nid = current_node()) -> void * {
            auto const object = do_allocate(gaf, nid);
            if (!object) {
                return nullptr;
            }
            return object;
        }

        auto do_deallocate(void *object) -> void {
            auto slab = role_cast<PfRole::Slab>(virt_to_folio(object));
            return do_deallocate(slab, object);
        }

        auto do_deallocate(Slab *slab, void *object) -> void;

        template <typename T>
        auto deallocate(T *object) -> void {
            DEBUG_ASSERT(object, "Pass a invalid object");
            ustl::mem::destroy_at(object);
            do_deallocate(reinterpret_cast<Object *>(object));
        }

        FORCE_INLINE
        auto object_size() const -> usize {
            return object_size_;
        }

        // Do not use it.
        auto init(char const *name, usize object_size, AlignVal align, OcFlags flags) -> Status;
        auto init_top_half(char const *name, usize object_size, AlignVal align, OcFlags flags) -> Status;
    private:
        auto init_cache_node() -> Status;
        auto init_cache_cpu() -> Status;
        auto parse_ocflags(OcFlags flags) -> Status;

        INIT_CODE
        auto init_cache_node_boot() -> Status;

        auto free_cache_node() -> void;

        auto get_slab(NodeId nid) -> Slab *;

        FORCE_INLINE
        auto alloc_slab(NodeId nid, Gaf gaf = {}) -> Slab * {
            return Slab::create(this, gaf_ | gaf, order_, object_size_, nid);
        }

        FORCE_INLINE
        auto create_slab(NodeId nid, Gaf gaf = {}) -> Slab * {
            auto slab = alloc_slab(nid, gaf);
            if (slab) {
                cache_node_[nid]->add_slab(slab);
            }

            return slab;
        }

        FORCE_INLINE
        auto free_slab(Slab *slab) -> void {
            return slab->destory();
        }

        GKTL_CANARY(ObjectCache, canary_);
        OcFlags ocflags_;
	    Gaf gaf_;
        u16 order_;
        u16 objects_;
	    u32 object_size_;
	    u32 object_align_;
	    u32 inuse_;
        u32 min_partial_;
	    const char *name_;
	    PerCpu<CacheOnCpu> cache_cpu_;
        ustl::Array<ObjectCachePerNode *, MAX_NODE> cache_node_;

        ustl::collections::intrusive::ListMemberHook<> managed_hook_;
        USTL_DECLARE_HOOK_OPTION(Self, managed_hook_, ManagedHook);

        enum class InitState {
            Bootstrap,
            Active,
        };
        static inline InitState s_state;
        static inline ustl::collections::intrusive::List<Self, ManagedHook>  s_oclist_;
    };

    template <typename Object>
    auto make_object_cache(char const *name, OcFlags ocflags) -> ustl::Rc<ObjectCache> {
        return ObjectCache::create(name, sizeof(Object), alignof(Object), ocflags) ;
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_OBJECT_CACHE_HPP
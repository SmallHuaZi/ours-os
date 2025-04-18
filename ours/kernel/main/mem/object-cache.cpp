#include <ours/mem/object-cache.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/mem/node-states.hpp>

namespace ours::mem {
    struct ObjectCache::CacheOnCpu {
        Slab *slab;
    };

    struct ObjectCache::CacheOnNode {
        /// Slabs which contains partial available objects.
        SlabList slabs_partial;
        /// Slabs which contains all objects inused.
        SlabList slabs_full;
        Mutex mutex_;
        usize num_slabs;
    };

    auto ObjectCache::free_slab(Slab *slab) -> void {
        DEBUG_ASSERT(slab->num_inuse == 0, "Attempt to free a frame inused.");
        auto frame = role_cast<PfRole::Pmm>(slab);
        mem::free_frame(frame, order_);

        // Trace the number of slabs on a node
        cache_node_[slab->nid()]->num_slabs -= 1;
    }

    FORCE_INLINE
    auto ObjectCache::get_neighbor(Object *object) -> Object * {
        return reinterpret_cast<Object *>(
            reinterpret_cast<u8 *>(object) + object_size_
        );
    }

    auto ObjectCache::alloc_slab(NodeId nid) -> Slab * {
        DEBUG_ASSERT(node_is_state(nid, NodeStates::Possible));

        auto frame = mem::alloc_frame(nid, gaf_, order_);
        if (!frame) {
            return nullptr;
        }
        auto slab = role_cast<PfRole::Slab>(frame);
        slab->num_inuse = 0;
        slab->num_objects = objects_;

        auto object = frame_to_virt<Object>(frame);
        for (auto i = 0; i < objects_; ++i) {
            slab->free_list.push_back(*object);
            object = get_neighbor(object);
        }

        // Trace the number of slabs on a node
        cache_node_[slab->nid()]->num_slabs += 1;
        return slab;
    }

    FORCE_INLINE
    auto ObjectCache::get_slab_from_node(NodeId nid) -> Slab * {
        DEBUG_ASSERT(node_is_state(nid, NodeStates::Possible));

        auto &node = *cache_node_[nid];
        SlabList *list = nullptr;
        if (!node.slabs_partial.empty()) {
            list = &node.slabs_partial;
        } 

        Slab *slab = nullptr;
        if(list) {
            slab = &list->front();
        }

        return slab;
    }

    auto ObjectCache::get_slab_any_node() -> Slab * {
        Slab *slab = nullptr;
        auto &nodes = node_online_mask();
        for (auto nid = 0; nid < nodes.size(); ++nid) {
            if (!nodes.test(nid)) {
                continue; 
            }

            slab = get_slab_from_node(nid);
            if (slab) {
                break;
            }
        }

        return slab;
    }

    FORCE_INLINE
    auto ObjectCache::get_slab(NodeId nid) -> Slab * {
        if (auto slab = get_slab_from_node(nid)) {
            return slab;
        }
        return get_slab_any_node();
    }

    auto ObjectCache::allocate(Gaf gaf, NodeId nid) -> Object * {
        // Alwasy look up the cache per cpu firstly.
        auto *slab = cache_cpu_.with_current([] (CacheOnCpu &cache) {
            return cache.slab;
        });
        if (!slab || nid != slab->nid() || !node_is_state(nid, NodeStates::Online)) {
            // A mismatched node, let us go girst to discard it and then attempt to in get_slab
            // re-search a slab whose'nid matched.
            slab = nullptr;
            nid = MAX_NODE;
        }

        if (!slab) {
            // Traverse all online nodes to re-search a nid-matched slab as much as possible.
            slab = get_slab(nid);
        }

        if (!slab) {
            // CPU and NODE cache is all empty, to allocate a slab.
            slab = alloc_slab(nid);
        }

        if (!slab) {
            return nullptr;
        }

        auto &object = slab->free_list.front();
        slab->free_list.pop_front();

        if (!slab->has_object()) {
            cache_node_[slab->nid()]->slabs_full.push_back(*slab);
            slab = nullptr;
        }

        cache_cpu_.with_current([slab] (CacheOnCpu &cache) {
            cache.slab = slab;
        });
        slab->num_inuse += 1;
        return &object;
    }

    auto ObjectCache::deallocate(Object *object) -> void {
        auto slab = role_cast<PfRole::Slab>(virt_to_frame(object));
        slab->free_list.push_back(*object);
        slab->num_inuse -= 1;
    }

} // namespace ours::mem
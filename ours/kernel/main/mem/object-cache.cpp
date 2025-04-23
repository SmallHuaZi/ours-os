#include <ours/mem/object-cache.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/mem/node-states.hpp>
#include <ours/mem/early-mem.hpp>

#include <ustl/lazy_init.hpp>
#include <logz4/log.hpp>
#include <ktl/new.hpp>

namespace ours::mem {
    static ObjectCache *s_object_cache_self;
    static ObjectCache *s_object_cache_node;

    INIT_DATA
    static ustl::LazyInit<ObjectCache> s_bootstrap_object_cache;

    INIT_DATA
    static ustl::LazyInit<ObjectCache> s_bootstrap_object_cache_node;

    FORCE_INLINE
    auto Slab::get_object() -> Object * {
        if (!has_object()) {
            return nullptr;
        }

        auto object = ustl::mem::address_of(free_list.front());
        free_list.pop_front();
        num_inuse += 1;
        return object;
    }

    auto Slab::init(ObjectCache *oc, usize order, usize obj_size) -> Status {
        num_inuse = 0;
        num_objects = BIT(order) * PAGE_SIZE / obj_size;

        auto object = frame_to_virt<Object>(to_pmm());
        for (auto i = 0; i < num_objects; ++i) {
            free_list.push_front(*object);
            object = reinterpret_cast<Object *>(
                reinterpret_cast<u8 *>(object) + obj_size
            );
        }

        object_cache = oc;

        return Status::Ok;
    }

    auto Slab::create(ObjectCache *oc, Gaf gaf, usize order, usize obi_size, NodeId nid) -> Slab * {
        auto frame = mem::alloc_frame(nid, gaf, order);
        if (!frame) {
            return nullptr;
        }
        auto slab = role_cast<PfRole::Slab>(frame);
        auto status = slab->init(oc, order, obi_size);
        if (Status::Ok != status) {
            mem::free_frame(frame, order);
            return nullptr;
        }

        return slab;
    }

    FORCE_INLINE
    auto ObjectCachePerNode::create(NodeId nid) -> Self * {
        auto self = new (*s_object_cache_node, kGafKernel, nid) ObjectCachePerNode();
        if (!self) {
            return nullptr;
        }
        ustl::mem::construct_at(self);

        return self;
    }

    FORCE_INLINE
    auto ObjectCachePerNode::destory() -> void {
        s_object_cache_node->deallocate(this);
    }

    struct ObjectCache::CacheOnCpu {
        Slab *slab;
    };

    auto ObjectCache::init_cache_cpu() -> Status {
        cache_cpu_ = CpuLocal::allocate<CacheOnCpu>();
        if (!cache_cpu_) {
            return Status::OutOfMem;
        }

        return Status::Ok;
    }

    auto ObjectCache::free_cache_node() -> void {
        for (auto node : cache_node_) {
            node->destory();
        }
    }

    INIT_CODE
    auto ObjectCache::init_cache_node_boot() -> Status {
        auto const &node = node_possible_mask();
        for (auto nid = 0; nid < node.size(); ++nid) {
            if (!node.test(nid)) {
                continue;
            }

            auto slab = alloc_slab(nid);
            if (!slab) {
                return Status::OutOfMem;
            }
            cache_node_[nid] = slab->get_object<ObjectCachePerNode>();
            ustl::mem::construct_at(cache_node_[nid]);
            cache_node_[nid]->add_slab(slab);
        }

        return Status::Ok;
    }

    auto ObjectCache::init_cache_node() -> Status {
        auto const &node = node_possible_mask();
        for (auto nid = 0; nid < node.size(); ++nid) {
            if (!node.test(nid)) {
                continue;
            }

            auto node = ObjectCachePerNode::create(nid);
            if (!node) {
                // Release all holding and report error.
                free_cache_node();
                return Status::OutOfMem;
            }

            cache_node_[nid] = node;
        }

        return Status::Ok;
    }

    auto ObjectCache::parse_ocflags(OcFlags flags) -> Status {
        return Status::Ok;
    }

    auto ObjectCache::init_top_half(char const *name, usize object_size, AlignVal align, OcFlags flags) -> Status {
        name_ = name;
        object_size_ = ustl::mem::align_up(object_size, sizeof(usize));
        order_ = 0;
        objects_ = (BIT(order_) * PAGE_SIZE) / object_size_;
        object_align_ = align;
        ocflags_ = flags;
        gaf_ = kGafKernel;
        return parse_ocflags(flags);
    }

    auto ObjectCache::init(char const *name, usize object_size, AlignVal align, OcFlags flags) -> Status {
        auto status = init_top_half(name, object_size, align, flags);
        if (Status::Ok != status) {
            return status;
        }

        status = init_cache_node();
        if (Status::Ok != status) {
            return status;
        }

        status = init_cache_cpu();
        if (Status::Ok != status) {
            return status;
        }

        s_oclist_.push_back(*this);
        return Status::Ok;
    }

    auto ObjectCache::create(const char *name, usize obj_size, AlignVal align, OcFlags flags) -> Self * {
        // In future we should check if the arguments is valid, but now it is unnecessary.
        auto self = new (*s_object_cache_self, kGafKernel) Self();
        if (!self) {
            log::trace("Failed to allcoate a object cache:[name: {}, os: {}, align: {}]", 
                name, obj_size, usize(align));
            return {};
        }

        auto status = self->init(name, obj_size, align, flags);
        if (Status::Ok != status) {
            log::trace("Failed to initialize a object cache:[name: {}, os: {}, align: {}]", 
                name, obj_size, usize(align));

            s_object_cache_self->deallocate(self);
            return {};
        }

        return self;
    }

    auto ObjectCache::create_boot(Self &self, char const *name, usize obj_size, AlignVal align, OcFlags flags) -> Status {
        auto status = self.init_top_half(name, obj_size, align, flags);
        if (status != Status::Ok) {
            return status;
        }

        status = self.init_cache_node_boot();
        if (status != Status::Ok) {
            return status;
        }

        status = self.init_cache_cpu();
        if (status != Status::Ok) {
            return status;
        }
        return Status::Ok;
    }

    FORCE_INLINE
    auto ObjectCache::get_slab(NodeId nid) -> Slab * {
        // Firstly, try on target node.
        if (nid != MAX_NODE && node_is_state(nid, NodeStates::Online)) {
            if (cache_node_[nid]->has_slab()) {
                if (auto slab = cache_node_[nid]->get_slab()) {
                    return slab;
                }
            }
        }

        // Try any nodes.
        Slab *slab = nullptr;
        auto &nodes = node_online_mask();
        for (auto nid = 0; nid < nodes.size(); ++nid) {
            if (!nodes.test(nid)) {
                continue; 
            }

            slab = cache_node_[nid]->get_slab();
            if (slab) {
                break;
            }
        }

        return slab;
    }

    auto ObjectCache::do_allocate(Gaf gaf, NodeId nid) -> Object * {
        if (!node_is_state(nid, NodeStates::Online)) {
            // The prefered node is offline, so the second predicate is invalid.
            nid = MAX_NODE;
        }

        // Alwasy look up the cache per cpu firstly.
        auto *slab = cache_cpu_.with_current([] (CacheOnCpu &cache) {
            return cache.slab;
        });
        if (slab) {
            if (nid != slab->nid() || nid != MAX_NODE) {
                // Mismatch node. We should let it then try to get slab on target node 
                // in another ways.
                slab = nullptr;
            }
        }

        if (!slab) {
            // Traverse all online nodes to re-search a nid-matched slab as much as possible.
            slab = get_slab(nid);
        }

        if (!slab) {
            // CPU and NODE cache have no available slabs, to allocate a new slab.
            slab = create_slab(nid, gaf);
        }

        if (!slab) {
            return nullptr;
        }

        auto object = slab->get_object();

        if (!slab->has_object()) {
            cache_node_[slab->nid()]->put_slab(slab);
            slab = nullptr;
        }

        cache_cpu_.with_current([slab] (CacheOnCpu &cache) {
            cache.slab = slab;
        });
        return object;
    }

    auto ObjectCache::do_deallocate(Slab *slab, void *object) -> void {
        DEBUG_ASSERT(slab);

        slab->put_object(static_cast<Object *>(object));
        slab->num_inuse -= 1;
        if (slab->num_inuse != 0) {
            return;
        }

        auto node = cache_node_[slab->nid()];
        if (node->num_partial >= min_partial_) {
            free_slab(slab);
        }
    }

    /// Requires that PMM is available.
    INIT_CODE
    auto init_object_cache() -> void {
        s_object_cache_self = s_bootstrap_object_cache.data();
        s_object_cache_node = s_bootstrap_object_cache_node.data();
        ObjectCache::create_boot(*s_object_cache_node, "node-cache", sizeof(ObjectCachePerNode), alignof(ObjectCachePerNode), {});
        ObjectCache::create_boot(*s_object_cache_self, "self-cache", sizeof(ObjectCache), alignof(ObjectCache), {});
    }

} // namespace ours::mem
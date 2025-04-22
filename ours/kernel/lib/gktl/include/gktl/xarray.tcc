#include <gktl/xarray.hpp>

#define TEMPLATE \
    template <typename Allocator, usize ChunkOrder>
#define XARRAY_CURSOR \
    Xarray<Allocator, ChunkOrder>::Cursor

namespace gktl {
    TEMPLATE 
    auto XARRAY_CURSOR::store(Entry value) -> Entry {
    }

    TEMPLATE 
    auto XARRAY_CURSOR::descend(NodePtrMut node) -> Entry {
        return node->get_entry(index_);
    }

    TEMPLATE 
    auto XARRAY_CURSOR::load() -> Entry {
        Entry entry = owner_->entry_;
        while (entry.is_node()) {
            NodePtrMut node = entry.node();
            // 
            if (shift_ > node->shift) {
                break;
            }

            entry = node->get_entry(index_);
            if (!node->shift) {
                // Leaf node.
                break;
            }
        }

        return entry;
    }

} // namespace gktl
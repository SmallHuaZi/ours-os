#include <ktl/xarray.hpp>

#define TEMPLATE \
    template <typename Allocator, typename Config>
#define XARRAY_CURSOR \
    Xarray<Allocator, Config>::Cursor

namespace ktl {
    TEMPLATE 
    auto XARRAY_CURSOR::expand(Entry head) -> ustl::Option<usize> {
        // Represent the shift of upper-level, by default
        usize shift = 0;
        usize max_idx = index_;

        if (head.is_null()) {
            // If given a null entry, just return the shift which is sufficient to
            // current index.
            while ((max_idx >> shift) >= kMaxSlots) {
                shift += kSlotsBits;
            }
            return ustl::some(shift);
        }

        NodePtrMut node = nullptr;
        if (head.is_node()) {
            // If a node, the shift of upper-level is shift of this node plus kSlotsBit.
            node = head.node();
            shift = node->shift + kSlotsBits;
        }

        // Bottom -> Up 
        while (max_idx > head.max_index()) {
            // Attempt to allocate parent node.
            node = owner_->alloc_node(shift);
            if (!node) {
                return ustl::none(); // Status::OutOfMem
            }
            if (head.is_value()) {
                node->nr_values += 1;
            }

            // Install this entry.
            node->set_entry(0, head);

            // Update for next iteration.
            head = Entry::make_node(node);

            // Update Xarray for correctly recording the root.
            owner_->payload_.entry = head;
            shift += kSlotsBits;
        }

        // Update cursor.
        node_ = node;
        return ustl::some(shift);
    }

    TEMPLATE 
    auto XARRAY_CURSOR::create_slots() -> ustl::Option<Entry> {
        Entry entry, *slot;
        usize shift, order = shift_;
        NodePtrMut node = node_;
        if (at_root()) {
            // If create slots from root of tree, we first have to attempt expanding the
            // height of teh current tree.
            entry = owner_->payload_.entry;
            shift = expand(entry);
            if (!shift) {
                return ustl::none();
            }
            // Because `expand` operation has changed the value of entry owner held possibly,
            // we have to update it.
            entry = owner_->payload_.entry;
            slot = &owner_->payload_.entry;
        } else if (node)  {
            shift = node->shift;
            entry = node->get_entry(index_);
            slot = &node_->slots[offset_];
        } else {
            shift = 0;
            entry = owner_->payload_.entry;
            slot = &owner_->payload_.entry;
        }

        // Top -> Down
        while (shift > order) {
            shift -= kSlotsBits;
            if (entry.is_null()) {
                node = owner_->alloc_node(shift);
                if (!node) {
                    break;
                }
                // Make lower level.
                *slot = Entry::make_node(node);
            } else if (entry.is_node()) {
                node = entry.node();
            } else {
                break;
            }

		    // Let's go for next layer.
            entry = descend(node);
            slot = &node->slots[offset_];
        }

        return entry;
    }

    TEMPLATE 
    auto XARRAY_CURSOR::store(Entry value) -> ktl::Result<Entry> {
        if (!value.is_null()) {
            create_slots();
        }
    }

    TEMPLATE FORCE_INLINE 
    auto XARRAY_CURSOR::descend(NodePtrMut node) -> Entry {
        offset_ = node->get_offset(index_);
        return node->get_entry(index_);
    }

    TEMPLATE 
    auto XARRAY_CURSOR::load() -> Entry {
        Entry entry = owner_->payload_.entry;
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

} // namespace ktl

#undef TEMPLATE
#undef XARRAY_CURSOR
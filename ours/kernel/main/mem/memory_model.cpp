#include <ours/mem/memory_model.hpp>
#include <ours/mem/vmm.hpp>
#include <ours/mem/pmm.hpp>
#include <ours/mem/pm_zone.hpp>
#include <ours/mem/early-mem.hpp>
#include <ours/mem/pm_node.hpp>
#include <ours/panic.hpp>

#include <ktl/new.hpp>
#include <logz4/log.hpp>
#include <ustl/algorithms/minmax.hpp>

namespace ours::mem {
    MemoryModel g_memory_model;

    template <typename T>
    FORCE_INLINE CXX11_CONSTEXPR
    static auto memory_model_allocate(usize n, AlignVal alignment, NodeId nid) -> T * {
        T *object;
        if (pmm_enabled()) {
            object = new (kGafKernel, nid) T[n];
        } else {
            object = EarlyMem::allocate<T>(n, alignment, nid);
            if (!object) {
                panic("Failed to allocate {} map", ustl::traits::IsSameV<T, PmFrame> ? "frame" : "section");
            }
        }

        // log::debug("{} map allocated at 0x{:8X}", ustl::traits::IsSameV<T, PmFrame> ? "Frame" : "Section", usize(object));
        return object;
    }

    static auto memory_model_dump_all_present(PmSection *section, usize level) -> void {
        char const *format_auxiliary_spaces[] = {
            "      ", "    ", "  ", ""
        };
        for (auto i = 0; i < MemoryModel::kEntriesPerSection; ++i) {
            if (!section[i].is_present()) {
                continue;
            }

            log::debug("{}L{}[{}]: {:X}", format_auxiliary_spaces[level + 1], level, i, usize(section));
            if (section[i].is_termianl()) {
                auto submap = section[i].frame_map().data();
                log::debug("{}F: {:X}", format_auxiliary_spaces[0], usize(submap));
            } else {
                auto submap = section[i].section_map().data();
                memory_model_dump_all_present(submap , level - 1);
            }
        }
    }

    auto MemoryModel::dump() -> void {
        log::debug("Section mapping information"); 
        memory_model_dump_all_present(sgd_, kTopLevel);
    }

    INIT_CODE
    auto MemoryModel::init(bool map_init) -> Status {
        sgd_ = memory_model_allocate<PmSection>(kEntriesPerSection, PAGE_SIZE, NodeId(0));
        if (!sgd_) {
            panic("Failed to allocate memory for root section");
        }

        global_node_states().for_each_possible([&] (NodeId nid) {
            EarlyMem::IterationContext context(bootmem::RegionType::Normal, nid);
            while (auto region = EarlyMem::iterate(context)) {
                add_range(region->base, region->end(), nid);
            }
        });

        if (map_init) {
            init_framemap();
        }
        return Status::Ok;
    }

    INIT_CODE
    auto MemoryModel::init_unavailable_framemap(Pfn start_pfn, Pfn end_pfn, ZoneType ztype, NodeId nid) -> void {
        for (auto pfn = start_pfn; pfn < end_pfn; ++pfn) {
            if (!exist(pfn)) {
                continue;
            }

            auto frame = pfn_to_frame(pfn);
            frame->init(ztype, pfn_to_secnum(pfn), nid);
            frame->mark_reserved();
        }
    }

    INIT_CODE
    auto MemoryModel::init_range_framemap(Pfn start_pfn, Pfn end_pfn, ZoneType ztype, NodeId nid) -> void {
        for (auto pfn = start_pfn; pfn < end_pfn; ++pfn) {
            auto frame = pfn_to_frame(pfn);
            DEBUG_ASSERT(frame, "");

            frame->init(ztype, pfn_to_secnum(pfn), nid);
        }
    }

    INIT_CODE
    auto MemoryModel::init_zone_framemap(PmZone &zone, Pfn start_pfn, Pfn end_pfn, Pfn hole_start_pfn) -> Pfn {
        auto const zone_start_pfn = zone.start_pfn();
        auto const zone_end_pfn = zone.start_pfn() + zone.spanned_frames();
        start_pfn = ustl::algorithms::clamp(start_pfn, zone_start_pfn, zone_end_pfn);
        end_pfn = ustl::algorithms::clamp(end_pfn, zone_start_pfn, zone_end_pfn);

        if (start_pfn >= end_pfn) {
            return start_pfn;
        }
        init_range_framemap(start_pfn, end_pfn, zone.zone_type(), zone.which_node());

        if (hole_start_pfn < start_pfn) {
            init_unavailable_framemap(hole_start_pfn, start_pfn, zone.zone_type(), zone.which_node());
        }
        return end_pfn;
    }

    INIT_CODE
    auto MemoryModel::init_framemap() -> void {
        auto hole_start_pfn = 0, start_pfn = 0, end_pfn = 0;
        EarlyMem::IterationContext context(bootmem::RegionType::Normal, MAX_NODE);
        while (auto region = EarlyMem::iterate(context)) {
            auto nid = region->nid();
            auto node = PmNode::node(nid);
            DEBUG_ASSERT(node != nullptr, "");

            start_pfn = phys_to_pfn(region->base);
            end_pfn = phys_to_pfn(region->end());
            for (auto zone: node->zone_queues()->get_queue(ZoneQueues::LocalContiguous)) {
                start_pfn = init_zone_framemap(*zone, start_pfn, end_pfn, hole_start_pfn);
                hole_start_pfn = end_pfn;
            }
        }

        // A gift for zero number of node.
        end_pfn = ustl::mem::align_up(end_pfn, kFramesPerLevel[0]);
        if (hole_start_pfn < end_pfn) {
            init_unavailable_framemap(hole_start_pfn, end_pfn, ZoneType::Normal, 0);
        }
    }

    auto MemoryModel::depopulate(Pfn start_pfn, Pfn end_pfn, NodeId nid) -> Status {
        auto start = reinterpret_cast<VirtAddr>(pfn_to_frame(start_pfn));
        auto end = reinterpret_cast<VirtAddr>(pfn_to_frame(end_pfn));

        return Status::Ok;
    }

    auto MemoryModel::init_index(usize level, SecNum secnum, NodeId nid) -> ustl::Result<PmSection *, Status> {
        auto section = sgd_;

        // Establish all of interminal indexies.
        for (isize i = kTopLevel; i > level; --i) {
            auto index = secnum_to_index(i, secnum);
            section = ustl::mem::address_of(section[index]);
            if (section->is_present()) {
                // There was a next level entry. Let us to observe next level directly.
                section = section->section_map().data();
            } else {
                // No sub section map, need we to create a new.
                auto submap = memory_model_allocate<PmSection>(kEntriesPerSection, PAGE_SIZE, nid);
                if (!submap) {
                    return ustl::err(Status::OutOfMem);
                }

                section->set_map(submap);
                section->set_level(i);
                section->mark_present();
                section->mark_interminal();
                section = submap;
            }
        }

        // Because of may existing discrete commits to multiple ranges in a same leaf section, we
        // have to determine whether the leaf has been marked with terminal.
        //
        // There are a part of potential optimization points possibly: 
        //   1) Normalizing the ranges in same section, make them was commited at once to avoid 
        //      re-commit.
        section = ustl::mem::address_of(section[secnum_to_index(level, secnum)]);
        if (!section->is_termianl()) {
            section->set_level(level);
            section->mark_terminal();
        }
        return ustl::ok(section);
    }

    auto MemoryModel::activate_section(PmSection *section, NodeId nid) -> ustl::Result<FrameMap, Status> {
        DEBUG_ASSERT(section, "");
        if (section->is_present()) {
            return ustl::ok(section->frame_map());
        }

        auto const nr_frames = kFramesPerLevel[section->level()];
        auto framemap = memory_model_allocate<PmFrame>(nr_frames, PAGE_SIZE, nid);
        if (!framemap) {
            return ustl::err(Status::OutOfMem);
        }

        section->set_map(framemap);
        section->mark_present();
        return ustl::ok(FrameMap(framemap, nr_frames));
    }

    template <typename F>
    auto MemoryModel::descend(SecNum begin, SecNum end, F &&f) -> Status {
        for (auto level = kMappingLevel; level > 0; --level) {
            for (auto this_end = begin + kLeavesPerLevel[level - 1]; this_end <= end; this_end += kLeavesPerLevel[level - 1]) {
                auto status = ustl::function::invoke(ustl::forward<F>(f), level - 1, begin);
                if (Status::Ok != status) {
                    return status;
                }
                begin = this_end;
            }

            if (begin == end) {
                break;
            }
        }

        return Status::Ok;
    }

    auto MemoryModel::add_range(PhysAddr start, PhysAddr end, NodeId nid) -> Status {
        auto process_range = [&]  (usize level, SecNum secbeg) -> Status {
            auto result = init_index(level, secbeg, nid);
            if (result.is_err()) {
                return result.unwrap_err();
            }
            auto section = result.unwrap();
            if (auto result = activate_section(section, nid); result.is_err()) {
                // FIXME(SmallHuaZi): We should mark this section as absent. Rather than directly return.
                return result.unwrap_err();
            }

            return Status::Ok;
        };

        auto const secnum_beg = phys_to_secnum(start);
        auto const secnum_end = phys_to_secnum(end) + 1;
        return descend(secnum_beg, secnum_end, process_range);
    }

    auto MemoryModel::mark_section(SecNum begin, SecNum end, PmSection::State state) 
        -> void {
        while (begin != end) {
            auto section = secnum_to_section(begin);
            section->set_state(state);
            begin += kLeavesPerLevel[section->level()];
        }
    }

} // namespace ours::mem
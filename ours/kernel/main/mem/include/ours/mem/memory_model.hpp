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
#ifndef OURS_MEM_MEMORY_MODEL_HPP
#define OURS_MEM_MEMORY_MODEL_HPP 1

#include <ours/init.hpp>
#include <ours/status.hpp>
#include <ours/assert.hpp>
#include <ours/macro_abi.hpp>
#include <ours/mem/types.hpp>
#include <ours/mem/pm_frame.hpp>
#include <ours/mem/physmap.hpp>

#include <ustl/array.hpp>
#include <ustl/bitfields.hpp>
#include <ustl/mem/address_of.hpp>
#include <ustl/traits/conditional.hpp>
#include <ustl/result.hpp>

namespace ours::mem {
    template <typename T>
    FORCE_INLINE CXX11_CONSTEXPR
    static auto phys_to_secnum(T phys_addr) -> SecNum {  
        static_assert(sizeof(T) == sizeof(PhysAddr));
        return reinterpret_cast<PhysAddr>(phys_addr) >> SECTION_SHIFT;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto secnum_to_phys(SecNum secnum) -> PhysAddr {  
        return secnum << SECTION_SHIFT;  
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto pfn_to_secnum(Pfn pfn) -> SecNum {  
        return pfn >> PFN_SECTION_SHIFT;
    }

    FORCE_INLINE CXX11_CONSTEXPR
    static auto secnum_to_pfn(SecNum secnum) -> Pfn {  
        return secnum << PFN_SECTION_SHIFT;
    }

    /// Leaf node
    struct PmSection {
        typedef ustl::views::Span<PmFrame>     FrameMap;
        typedef ustl::views::Span<PmSection>   SecMap;

        enum class State {
            None,
            Present,
            Online,
            MaxNumState,
        };

        enum class Type {
            UnknownType,
            Terminal,
            Interminal,
            MaxNumType,
        };

        enum FieldId {
            StateId,
            TypeId,
            LevelId,
            MapId,
        };

        CXX11_CONSTEXPR
        static auto const kMaxLevel = 3;

        FORCE_INLINE CXX11_CONSTEXPR
        auto state() const -> State {
            return value_.get<StateId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_state(State state) -> void {
            value_.set<StateId>(state);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mark_present() -> void {
            value_.set<StateId>(State::Present);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_present() const -> bool {
            return state() == State::Present;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_online() const -> bool {
            return state() == State::Online;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto is_termianl() const -> bool {
            return type() == Type::Terminal;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto type() const -> Type {
            return value_.get<TypeId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_type(Type type) -> void {
            value_.set<TypeId>(type);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mark_terminal() -> void {
            value_.set<TypeId>(Type::Terminal);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto mark_interminal() -> void {
            value_.set<TypeId>(Type::Interminal);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto level() const -> usize {
            return value_.get<LevelId>();
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_level(usize level) -> void {
            value_.set<LevelId>(level);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto frame_map() const -> FrameMap {
            return { reinterpret_cast<PmFrame *>(value_.get<MapId>()), PAGE_SIZE / sizeof(FrameMap *) };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto section_map() const -> SecMap {
            return { reinterpret_cast<PmSection *>(value_.get<MapId>()), PAGE_SIZE / sizeof(*this) };
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_map(void *map) -> void {
            value_.set<MapId>(map);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto has_submap() const -> bool {
            return value_.get<MapId>();
        }

        template <FieldId Id, typename Type, usize Bits, bool Natural = false>
        using Field =ustl::Field<ustl::bitfields::Id<Id>, 
                                 ustl::bitfields::Type<Type>, 
                                 ustl::bitfields::Bits<Bits>,
                                 ustl::bitfields::Natural<Natural>>;

        using EncodedUsize = ustl::BitFields<ustl::bitfields::StorageUnit<usize>, ustl::TypeList<
            Field<StateId, State, ustl::bit_width(usize(State::MaxNumState))>,
            Field<TypeId, Type, ustl::bit_width(usize(Type::MaxNumType))>,
            Field<LevelId, usize, ustl::bit_width<usize>(kMaxLevel)>,
            // FIXME(SmallHuaZi): Hard code.
            Field<MapId, void *, 58, true>
        >>;
        EncodedUsize value_;
    };

    /// `MemoryModel` is a high-level abstract and implementation to physical memory layout.
    /// It's responsibility is to pre-allocate `PmFrame` and to provide a group of the convertion
    /// method among `PmFrame`, `Pfn`, `SecNum`, `VirtAddr` and `PhysAddr`.
    class MemoryModel {
        typedef MemoryModel       Self;
        typedef ustl::views::Span<PmFrame>     FrameMap;
        typedef ustl::views::Span<PmSection>   SecMap;
    public:
        CXX11_CONSTEXPR
        static usize const kMappingLevel = PmSection::kMaxLevel;

        CXX11_CONSTEXPR
        static usize const kTopLevel = kMappingLevel - 1;

        CXX11_CONSTEXPR
        static usize const kEntriesPerSection = PAGE_SIZE / sizeof(PmSection);

        CXX11_CONSTEXPR
        static auto const kLeavesPerLevel = [] <usize... Levels> (ustl::IndexSequence<Levels...>) {
            static_assert(sizeof...(Levels) == kMappingLevel);
            ustl::Array<usize, sizeof...(Levels)> array;
            // L0 entry is a leaf itself.
            array[0] = 1;
            for (auto i = 1; i < array.size(); ++i) {
                array[i] = array[i - 1] * kEntriesPerSection;
            }
            return array;
        } (ustl::MakeIndexSequenceT<kMappingLevel>());

        CXX11_CONSTEXPR
        static auto const kFramesPerLevel = [] <usize... Levels> (ustl::IndexSequence<Levels...>) {
            static_assert(sizeof...(Levels) == kMappingLevel);
            ustl::Array<usize, sizeof...(Levels)> array;
            array[0] = BIT(SECTION_SHIFT - PAGE_SHIFT);
            for (auto i = 1; i < array.size(); ++i) {
                array[i] = array[i - 1] * kEntriesPerSection;
            }
            return array;
        } (ustl::MakeIndexSequenceT<kMappingLevel>());

        CXX11_CONSTEXPR
        static usize const kIndexBits = {
            (ustl::NumericLimits<usize>::DIGITS - SECTION_SIZE_BITS + kMappingLevel) / kMappingLevel
        };

        MemoryModel() = default;
        ~MemoryModel() = default;

        /// Can not copy and move.
        MemoryModel(Self &&) = delete;
        MemoryModel(Self const &) = delete;

        auto operator=(Self &&) -> Self & = delete;
        auto operator=(Self const &) -> Self & = delete;

        INIT_CODE
        auto init(bool map_init) -> Status;

        /// Estimate whether the frame descriptor of given pfn exist. It is do not means that 
        /// the memory that descriptor represents is available (may hole).
        auto exist(Pfn pfn) const -> bool {
            auto section = pfn_to_section(pfn);
            if (!section) {
                return false;
            }

            return section->is_present();
        }

        auto exist(Pfn start, Pfn end) const -> bool;

        FORCE_INLINE CXX11_CONSTEXPR
        auto pfn_to_section(Pfn pfn) const -> PmSection * {
            return secnum_to_section(pfn_to_secnum(pfn));
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto secnum_to_section(SecNum secnum) const -> PmSection * {
            return priv_secnum_to_section(secnum);
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto pfn_to_frame(Pfn pfn) -> PmFrame * {
            if (auto const section = secnum_to_section(pfn_to_secnum(pfn))) {
                return ustl::mem::address_of(section->frame_map()[pfn & PFN_SECTION_MASK]);
            }

            return nullptr;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto frame_to_pfn(PmFrame const *frame) -> Pfn {
            DEBUG_ASSERT(frame != nullptr);
            if (auto const section = frame_to_section(frame)) {
                return secnum_to_pfn(frame->secnum()) + (frame - section->frame_map().data());
            }

            return 0;
        }

        FORCE_INLINE CXX11_CONSTEXPR 
        auto frame_to_section(PmFrame const *frame) -> PmSection * {
            DEBUG_ASSERT(frame != nullptr);
            return secnum_to_section(frame->secnum());
        }

        auto add_range(PhysAddr start, PhysAddr end, NodeId nid) -> Status;

        auto remove_range(PhysAddr start, PhysAddr end, NodeId nid) -> Status;

        INIT_CODE
        auto init_framemap() -> void;

        auto dump() -> void;

    private:
        template <typename F>
        auto dispatch_range_by_hierarchy(SecNum start, SecNum end, F &&f) -> Status;

        auto init_index(usize level, SecNum begin, NodeId nid) -> ustl::Result<PmSection *, Status>;

        FORCE_INLINE
        auto activate_section(SecNum secnum, NodeId nid) -> ustl::Result<FrameMap, Status> {
            auto section = secnum_to_section(secnum);
            if (!section) {
                return ustl::err(Status::OutOfMem);
            }

            return activate_section(section, nid);
        }

        auto activate_section(PmSection *section, NodeId nid) -> ustl::Result<FrameMap, Status>;

        /// Initialize the page frames in the overlapping regions between [ `|start_pfn|`, `|end_pfn|`) and 
        /// the `|zone|` managed memory regions.
        ///
        /// Return the pfn of the first frame which not be initialized.
        INIT_CODE
        auto init_zone_framemap(PmZone &zone, Pfn start_pfn, Pfn end_pfn, Pfn hole_start_pfn) -> Pfn;

        INIT_CODE
        auto init_range_framemap(Pfn start_pfn, Pfn end_pfn, ZoneType ztype, NodeId nid) -> void;

        INIT_CODE
        auto init_unavailable_framemap(Pfn start_pfn, Pfn end_pfn, ZoneType ztype, NodeId nid) -> void;

        auto depopulate(Pfn start, usize nr_frames, NodeId nid) -> Status;

        auto mark_section(SecNum begin, SecNum end, PmSection::State) -> void;

        FORCE_INLINE CXX11_CONSTEXPR
        static auto secnum_to_index(usize level, SecNum secnum) -> usize {
            CXX11_CONSTEXPR
            static auto const kShift = [] () {
                ustl::Array<usize, kMappingLevel> array;
                array[0] = 0;
                for (auto i = 1; i < array.size(); ++i) {
                    array[i] = array[i - 1] + kIndexBits;
                }
                return array;
            } ();

            return (secnum >> kShift[level]) & (BIT(kIndexBits) - 1);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto priv_secnum_to_section(SecNum secnum) const -> PmSection * {
            DEBUG_ASSERT(sgd_);

            /// A short loop will be unfold.
            PmSection *section = sgd_;
            for (isize level = kMappingLevel - 1; level >= 0; --level) {
                section = ustl::mem::address_of(section[secnum_to_index(level, secnum)]);
                if (!section->is_present()) {
                    return nullptr;
                }
                if (section->is_termianl()) {
                    return section;
                }
                section = section->section_map().data();
            }

            return section;
        }

        PmSection *sgd_;
    };

    /// Accessors for conveniency.

    FORCE_INLINE
    static auto global_memory_model() -> MemoryModel & {
        extern MemoryModel g_memory_model;
        return g_memory_model;
    }

    FORCE_INLINE
    static auto init_memory_model(bool map_init) -> void {
        global_memory_model().init(map_init);
    }

    FORCE_INLINE
    static auto pfn_to_frame(Pfn pfn) -> PmFrame * {
        return global_memory_model().pfn_to_frame(pfn);
    }

    template <typename T>
    FORCE_INLINE
    static auto phys_to_frame(T phys_addr) -> PmFrame * {
        return pfn_to_frame(phys_to_pfn(phys_addr));
    }

    FORCE_INLINE
    static auto frame_to_pfn(PmFrame const *frame) -> Pfn {
        return global_memory_model().frame_to_pfn(frame);
    }

    FORCE_INLINE
    static auto frame_to_phys(PmFrame const *frame) -> PhysAddr {
        return pfn_to_phys(frame_to_pfn(frame));
    }

    template <typename T>
    FORCE_INLINE
    static auto virt_to_frame(T virt_addr) -> PmFrame * {
        return phys_to_frame(PhysMap::virt_to_phys(virt_addr));
    }

    FORCE_INLINE
    static auto frame_to_virt(PmFrame *frame) -> VirtAddr {
        return PhysMap::phys_to_virt(frame_to_phys(frame));
    }

    template <typename T>
    FORCE_INLINE
    static auto frame_to_virt(PmFrame *frame) -> T * {
        DEBUG_ASSERT(frame->phys_size() > sizeof(T), "T is too large");
        return PhysMap::phys_to_virt<T>(frame_to_phys(frame));
    }

    template <typename T>
    FORCE_INLINE
    static auto virt_to_folio(T virt_addr) -> PmFolio * {
        return frame_to_folio(
            phys_to_frame(PhysMap::virt_to_phys(virt_addr))
        );
    }

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_MODEL_HPP
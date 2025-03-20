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

#include <ours/status.hpp>
#include <ours/macro_abi.hpp>
#include <ours/mem/types.hpp>

#include <ustl/bitfields.hpp>

namespace ours::mem {
namespace details {
    using ustl::bitfields::Id;
    using ustl::bitfields::Bits;
    using ustl::bitfields::Name;
    using ustl::bitfields::Type;

    USTL_CONSTEXPR
    static usize const USIZE_BITS = ustl::NumericLimits<usize>::DIGITS;

    enum State {
        Present,
        Online,
        MaxNumState,
    };
    CXX11_CONSTEXPR
    static auto const STATE_BW = ustl::bit_width<usize>(MaxNumState);

    typedef ustl::Field<Id<0>, Type<State>, Bits<STATE_BW>>  ControlField;
    typedef ustl::Field<Id<1>, Type<PmFrame *>, Bits<USIZE_BITS - STATE_BW>> FrameMapField;

    typedef ustl::BitFields<ustl::TypeList<FrameMapField, ControlField>> EncodedUsize;
    static_assert(sizeof(EncodedUsize) == sizeof(usize), "This is not the thing we expect.");

    /// Leaf node
    struct PmSection {
        typedef ustl::views::Span<PmFrame>  FrameMap;

        enum FieldId {
            StateId,
            FrameMapId,
        };

        auto state() const -> State {
            return value_.get<StateId>();
        }

        auto set_state() const -> State {
            return value_.get<StateId>();
        }

        auto frame_map() const -> FrameMap {
            auto const raw_frame_map = value_.get<FrameMapId>();
            return { raw_frame_map, 0 };
        }

        EncodedUsize value_;
    };

} // namespace ours::mem::details

    /// `MemoryModel` is a high-level abstract and implementation to physical memory layout.
    /// It's responsibility is to pre-allocate `PmFrame` and to provide a group of the convertion
    /// method among `PmFrame`, `Pfn`, `VirtAddr` and `PhysAddr`.
    class MemoryModel
    {
        typedef MemoryModel       Self;
    public:
        static auto init() -> Status;

        static auto exist(Pfn frame) -> bool;

        static auto exist(Pfn start, Pfn end) -> bool;

        static auto phys_to_frame(PhysAddr phys_addr) -> PmFrame *
        {  return Self::pfn_to_frame(phys_to_pfn(phys_addr));  }

        static auto frame_to_phys(PmFrame *frame) -> PhysAddr
        {  return pfn_to_phys(Self::frame_to_pfn(frame));  }

        static auto pfn_to_frame(Pfn pfn) -> PmFrame *
        {  return 0; }

        static auto virt_to_frame(VirtAddr virt) -> PmFrame *
        {  return 0; }

        static auto frame_to_pfn(PmFrame *frame) -> Pfn
        {  return 0; }

        static auto mark_present(Pfn start_pfn, Pfn end_pfn) -> void;

        static auto populate(Pfn start_pfn, Pfn end_pfn) -> Status;

        static auto populate(Pfn start, usize nr_frames, NodeId nid, Altmap *altmap) -> Status;

        static auto depopulate(Pfn start, usize nr_frames) -> Status;
    private:
        static details::PmSection *ste_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_MODEL_HPP
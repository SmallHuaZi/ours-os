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
#include <ours/marco_abi.hpp>
#include <ours/mem/types.hpp>

#include <ustl/bitfields.hpp>

namespace ours::mem {
    /// Leaf node
    struct PmSection
    {
        enum State {
            Present,
            Online,
        };

        USTL_CONSTEXPR
        static usize const USIZE_BITS = ustl::NumericLimits<usize>::DIGITS;

        typedef ustl::BitField<
            ustl::FieldId<0>,
            ustl::FieldType<State>, 
            ustl::FieldBits<SECTION_SHIFT>
        > ControlField;

        typedef ustl::BitField<
            ustl::FieldId<1>,
            ustl::FieldType<PmFrame *>, 
            ustl::FieldBits<USIZE_BITS - SECTION_SHIFT>
        > FrameMapField;

        typedef ustl::BitFields<ustl::TypeList<FrameMapField, ControlField>> EncodedUsize;
        static_assert(sizeof(EncodedUsize) == sizeof(usize), "This is not the thing we expect.");

        EncodedUsize value_;
    };

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
        static PmSection *ste_;
    };

} // namespace ours::mem

#endif // #ifndef OURS_MEM_MEMORY_MODEL_HPP
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
#ifndef ARCH_X86_DESCRIPTOR_HPP
#define ARCH_X86_DESCRIPTOR_HPP 1

#include <arch/types.hpp>

#include <ustl/bitfields.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/util/index_sequence.hpp>
#include <ustl/util/enum_bits.hpp>

namespace arch {
    // These raw fields are normally accessed via the accessors defined below.
    enum class SegType: u8 {
        // For non-system segments
        AccessedBit = 0b00001,
        ReadOrWriteBit = 0b00010,
        DirectionOrConformingBit = 0b00100,
        ExecutableBit = 0b01000,

        // If set, the segment is a non-system segment.
        SystemBit = 0b10000,

        DataRO = SystemBit,
        DataRW = SystemBit | ReadOrWriteBit,
        DataRODown = DataRO | DirectionOrConformingBit,
        DataRWDown = DataRW | DirectionOrConformingBit,
        CodeXO = SystemBit | ExecutableBit,
        CodeRX = CodeXO | ReadOrWriteBit,
        CodeXOConforming = CodeXO | DirectionOrConformingBit,
        CodeRXConforming = CodeRX | DirectionOrConformingBit,

        Tss64Available = 0x9,
        Tss64Busy = 0xB,
    };
    USTL_ENABLE_ENUM_BITMASK(SegType);

    FORCE_INLINE
    static auto to_string(SegType type) -> char const * {
        switch (type) {
            case SegType::DataRO: return "DataRO";
            case SegType::DataRW: return "DataRW";
            case SegType::DataRODown: return "DataRODown";
            case SegType::DataRWDown: return "DataRWDown";
            case SegType::CodeXO: return "CodeXO";
            case SegType::CodeRX: return "CodeRX";
            case SegType::CodeXOConforming: return "CodeXOConforming";
            case SegType::CodeRXConforming: return "CodeRXConforming";
            case SegType::Tss64Available: return "TssAvailable";
            case SegType::Tss64Busy: return "TssBusy";
            default: return "Unknown";
        }
    }

    enum class Dpl: u8 {
        Ring0,
        Ring1,
        Ring2,
        Ring3,
    };

    FORCE_INLINE
    static auto to_string(Dpl dpl) -> char const * {
        switch (dpl) {
            case Dpl::Ring0: return "Ring0";
            case Dpl::Ring1: return "Ring1";
            case Dpl::Ring2: return "Ring2";
            case Dpl::Ring3: return "Ring3";
            default: return "Unknown";
        }
    }

    enum class SegGran: u8 {
        Byte = 0b0,
        Page = 0b1,
    };

namespace details {
    using ustl::Field;
    using ustl::BitFields;
    using ustl::bitfields::Id;
    using ustl::bitfields::Enable;
    using ustl::bitfields::Bits;
    using ustl::bitfields::Type;
    using ustl::traits::IsSameV;
    using ustl::traits::ConditionalT;
    using ustl::IntegerSequencePushBackT;

    struct Desc32Tag {};
    struct Desc64Tag {};

    // This represents the 32-bit descriptor format in the GDT or LDT.
    template <typename Tag>
    struct GdtDescImpl {
        typedef GdtDescImpl  Self;

        CXX11_CONSTEXPR
        static auto const Is64BitsV = IsSameV<Tag, Desc64Tag>;

        enum FieldId {
            LimitLow16Id,
            BaseLow16Id,
            BaseMid8Id,
            TypeId,
            DplId,
            PresentId,
            LimitHigh4Id,
            Reserved1BitId,
            LongModeId,
            Addr32Id,
            GranularityId,
            BaseHigh8Id,
            BaseHigh32Id,
            Reserved32Id,
        };

        typedef ustl::TypeList<
            Field<Id<LimitLow16Id>, Bits<16>>,
            Field<Id<BaseLow16Id>, Bits<16>>,
            Field<Id<BaseMid8Id>, Bits<8>>,
            // Access bytes
            Field<Id<TypeId>, Bits<5>, Type<SegType>>,
            Field<Id<DplId>, Bits<2>, Type<Dpl>>,
            Field<Id<PresentId>, Bits<1>>,
            //
            Field<Id<LimitHigh4Id>, Bits<4>>,
            // Flags
            Field<Id<Reserved1BitId>, Bits<1>>,
            Field<Id<LongModeId>, Bits<1>>,
            Field<Id<Addr32Id>, Bits<1>>,
            Field<Id<GranularityId>, Bits<1>, Type<SegGran>>,
            //
            Field<Id<BaseHigh8Id>, Bits<8>>,
            Field<Id<BaseHigh32Id>, Bits<32>, Enable<Is64BitsV>>,
            Field<Id<Reserved32Id>, Bits<32>, Enable<Is64BitsV>>
        > FieldList;

        typedef ustl::IndexSequence<BaseLow16Id, BaseMid8Id, BaseHigh8Id>  BaseField32;
        typedef ConditionalT<Is64BitsV, IntegerSequencePushBackT<BaseField32, BaseHigh32Id>, BaseField32>  BaseField;

        typedef ustl::IndexSequence<LimitLow16Id, LimitHigh4Id>  LimitField;

        BitFields<FieldList>  values;

        CXX11_CONSTEXPR
        auto set_present(bool present) -> Self & {
            values.template set<PresentId>(present);
            return *this;
        }

        CXX11_CONSTEXPR
        auto is_present() const -> bool {
            return values.template get<PresentId>();
        }

        // Get/set the 32-bit base address, splitting/combining its three fields.
        CXX11_CONSTEXPR
        auto base() const -> usize {
            return values.template pack<usize>(BaseField());
        }

        CXX11_CONSTEXPR
        auto set_base(usize base) -> Self & {
            values.template unpack<usize>(base, BaseField());
            return *this;
        }

        CXX11_CONSTEXPR
        auto limit() const -> usize {
            return values.template pack<usize>(LimitField());
        }

        CXX11_CONSTEXPR
        auto set_limit(usize limit) -> Self & {
            values.template unpack<usize>(limit, LimitField());
            return *this;
        }

        CXX11_CONSTEXPR
        auto type() const -> SegType {
            return values.template get<TypeId>();
        }

        CXX11_CONSTEXPR
        auto set_type(SegType type) -> Self & {
            values.template set<TypeId>(type);
            return *this;
        }

        CXX11_CONSTEXPR
        auto dpl() const -> Dpl {
            return values.template get<DplId>();
        }

        CXX11_CONSTEXPR
        auto set_dpl(Dpl dpl) -> Self & {
            values.template set<DplId>(dpl);
            return *this;
        }

        CXX11_CONSTEXPR
        auto granularity() const -> bool {
            return values.template get<GranularityId>();
        }

        CXX11_CONSTEXPR
        auto set_granularity(SegGran gran) -> Self & {
            values.template set<GranularityId>(gran);
            return *this;
        }

        CXX11_CONSTEXPR
        auto is_accessed() const -> bool {
            return !!(type() & SegType::AccessedBit);
        }

        CXX11_CONSTEXPR
        auto set_accessed(bool accessed) -> Self & {
            values.template set<TypeId>(type() | SegType::AccessedBit);
            return *this;
        }

        CXX11_CONSTEXPR
        auto is_longmode() const -> bool {
            return values.template get<LongModeId>();
        }

        CXX11_CONSTEXPR
        auto set_longmode(bool enable) -> Self & {
            values.template set<LongModeId>(enable);
            return *this;
        }

        CXX11_CONSTEXPR
        auto make_flat() -> Self & {
            set_base(0).set_limit(~0).set_present(true);
            return *this;
        }

        CXX11_CONSTEXPR
        auto make_flat_code() -> Self & {
            make_flat().set_type(SegType::CodeRX);
            return *this;
        }
    };
    typedef GdtDescImpl<Desc32Tag>     GdtDesc32;
    static_assert(sizeof(GdtDesc32) == 8);

    typedef GdtDescImpl<Desc64Tag>     GdtDesc64;
    static_assert(sizeof(GdtDesc64) == 16);

} // namespace details

    using details::GdtDesc32;
    using details::GdtDesc64;

    typedef ustl::traits::ConditionalT<sizeof(usize) == 8, GdtDesc64, GdtDesc32>
        GdtDesc;

    // A 64-bit system segment.
    //
    // These descriptors are used in 64-bit mode for system segments, call gates,
    // interrupt gates, and trap gates.
    //
    // Code and data segment descriptors continue to use the 32-bit descriptor
    // Desc32 format above.
    //
    // When used in the GDT or LDT, these 64-bit descriptors occupy two slots
    // in the table.
    //
    // [amd/vol2]: Section 4.8.3. System Descriptors
    // [intel/vol3]: Figure 7-4. Format of TSS and LDT Descriptors in 64-bit Mode
    struct SystemSegmentDesc64 {
    };
    // static_assert(sizeof(SystemSegmentDesc64) == 16);

    // Pointer/limit to the system GDT and IDT.
    //
    // If user mode alignment checks are enabled the struct needs to be aligned
    // such that (ptr % 4 == 2), which can be done using `AlignedGdtRegister64`
    // below. Privileged mode users or users with alignment checks disabled need
    // not worry. (c.f., [intel/vol3] Section 3.5.1 Segment Descriptor Tables)
    //
    // [intel/vol3]: Figure 2-6. Memory Management Registers
    // [amd/vol2]: Figure 4-8. GDTR and IDTR Format-Long Mode.
    struct PACKED DescPtr {
        template <typename T, usize N>
        static DescPtr make(T (&entries)[N]) {
            return {
                .limit = static_cast<u16>(sizeof(T) * N - 1),
                .base = reinterpret_cast<u64>(&entries),
            };
        }

        u16 limit; // Size of the GDT in bytes, minus one.
        usize base;  // Pointer to the GDT.
    };

    // Wrapper around GdtRegister64 to ensure the inner GdtRegister64 is correctly
    // aligned as described above.
    struct PACKED AlignedDescPtr {
        uint8_t padding[6];
        DescPtr reg;

        explicit AlignedDescPtr(DescPtr reg)
            : reg(reg) {
        }
    };
    static_assert(sizeof(AlignedDescPtr) == 16);

    // x86-64 Task State Segment.
    //
    // In 64-bit mode, the system needs at least one TSS. It is used to store
    // stack pointers for various privilege levels, stack pointers for various
    // interrupt handlers, and I/O port permissions.
    //
    // [amd/vol2]: Figure 12-8. Long Mode TSS Format
    // [intel/vol3]: Figure 7-11. 64-Bit TSS Format
    struct PACKED TaskStateSegment64 {
        u32 reserved0;

        // Stack pointers for various privilege levels.
        u64 sp0;
        u64 sp1;
        u64 sp2;

        u32 reserved1;
        u32 reserved2;

        // Interrupt stack table pointers.
        //
        // Note that ist[0] corresponds to IST1, i.e. the ist field in an
        // INTERRUPT_GATE descriptor is 1-origin though this array is 0-origin.
        // Descriptors with an ist field of zero do not use the IST at all.
        static constexpr size_t kNumInterruptStacks = 7;
        u64 ist[kNumInterruptStacks];

        u32 reserved3;
        u32 reserved4;
        u16 reserved5;

        // 16-bit offset of the I/O port permission map from the base of this structure.
        //
        // The bitmap will typically directly follow this structure directly, but the
        // `io_port_bitmap_base` allows for some other data to come prior to the bitmap.
        //
        // Access is granted for an I/O operation if all bits associated with the
        // read/write are clear. For example, a 2-byte write to port 0x80 will
        // require bits 0x80 and 0x81 to be clear.
        //
        // The CPU may read up to 1 byte past the limit specified, so an additional
        // padding byte of '0xff' should follow the bitmap. (See [amd/vol2] Section
        // 12.2.4, "I/O-Permission Bitmap").
        static constexpr u32 kMaxIoBitmapBits = 65536;
        static constexpr u32 kMaxIoBitmapBytes = 8192;
        u16 io_port_bitmap_base;
    };

    FORCE_INLINE
    static auto lgdt(DescPtr desc) -> void {
        asm volatile("lgdt %0" :: "m"(desc) : "memory");
    }

    FORCE_INLINE
    static auto lidt(DescPtr desc) -> void {
        asm volatile("lidt %0" :: "m"(desc) : "memory");
    }

    /// If `selector` is a NULL selector
    ///     THEN #GP(0);
    /// IF SRC(Offset) > descriptor table limit OR IF SRC(type) ≠ global
    ///     THEN #GP(segment selector); FI;
    /// Read segment descriptor;
    /// IF segment descriptor is not for an available TSS
    ///     THEN #GP(segment selector); FI;
    /// IF segment descriptor is not present
    ///     THEN #NP(segment selector); FI;
    /// TSSsegmentDescriptor(busy) := 1;
    FORCE_INLINE
    static auto ltr(u16 selector) -> void {
        asm volatile("ltr %0" :: "r"(selector) : "memory");
    }

} // namespace arch

#endif // #ifndef ARCH_X86_DESCRIPTOR_HPP
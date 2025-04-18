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

namespace arch {
    // These raw fields are normally accessed via the accessors defined below.
    enum class SegType: u8 {
        DataRO = 0b000,
        DataRW = 0b001,
        DataRODown = 0b010,
        DataRWDown = 0b011,
        CodeXO = 0b100,
        CodeRX = 0b101,
        CodeXOConforming = 0b110,
        CodeRXConforming = 0b111,
        System = 0b1000,
    };

    enum class Dpl: u8 {
        Ring0,
        Ring1,
        Ring2,
        Ring3,
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
            AccessedId,
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
            Field<Id<AccessedId>, Bits<1>>,
            Field<Id<TypeId>, Bits<4>, Type<SegType>>, 
            Field<Id<DplId>, Bits<2>, Type<Dpl>>,
            Field<Id<PresentId>, Bits<1>>,
            //
            Field<Id<LimitHigh4Id>, Bits<4>>, 
            // Flags
            Field<Id<Reserved1BitId>, Bits<1>>,
            Field<Id<LongModeId>, Bits<1>>,
            Field<Id<Addr32Id>, Bits<1>>,
            Field<Id<GranularityId>, Bits<1>>,
            //
            Field<Id<BaseHigh8Id>, Bits<8>>,
            Field<Id<BaseHigh32Id>, Bits<32>, Enable<Is64BitsV>>,
            Field<Id<Reserved32Id>, Bits<32>, Enable<Is64BitsV>>
        > FieldList;

        typedef ustl::IndexSequence<BaseLow16Id, BaseMid8Id, BaseHigh8Id>  BaseField32;
        typedef ConditionalT<Is64BitsV, IntegerSequencePushBackT<BaseField32, BaseHigh32Id>, BaseField32>  BaseField;

        typedef ustl::IndexSequence<BaseLow16Id, BaseMid8Id, BaseHigh8Id>  LimitField;

        BitFields<FieldList>  values;

        CXX11_CONSTEXPR
        auto set_present(bool state) -> Self & {
            values.template set<PresentId>(state);
            return *this;
        }

        CXX11_CONSTEXPR
        auto is_present() const -> bool {
            return values.template get<PresentId>();
        }

        // Get/set the 32-bit base address, splitting/combining its three fields.
        CXX11_CONSTEXPR
        auto base() const -> u32 {
            return values.template pack<u32>(BaseField());
        }

        CXX11_CONSTEXPR
        auto set_base(u32 base) -> Self & {
            values.template unpack<u32>(base, BaseField());
            return *this;
        }

        CXX11_CONSTEXPR
        auto limit() const -> u32 {
            return values.template pack<u32>(LimitField());
        }

        CXX11_CONSTEXPR
        auto set_limit(u32 limit) -> Self & {
            values.template unpack<u32>(limit, LimitField());
            return *this;
        }

        CXX11_CONSTEXPR
        auto type(SegType type) -> SegType {
            values.template set<TypeId>(type);
            return *this;
        }

        CXX11_CONSTEXPR
        auto set_type(SegType type) -> Self & {
            values.template set<TypeId>(type);
            return *this;
        }

        CXX11_CONSTEXPR
        auto dpl(Dpl) -> Dpl {
            return values.template get<DplId>();
        }

        CXX11_CONSTEXPR
        auto set_dpl(Dpl dpl) -> Self & {
            values.template set<DplId>(dpl);
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
    struct PACKED DescPtr64 {
        template <typename T, usize N>
        static DescPtr64 make(T (&entries)[N]) {
            return {
                .limit = static_cast<u16>(sizeof(T) * N - 1),
                .base = reinterpret_cast<u64>(&entries),
            };
        }

        u16 limit; // Size of the GDT in bytes, minus one.
        u64 base;  // Pointer to the GDT.
    };
    static_assert(sizeof(DescPtr64) == 10);

    // Wrapper around GdtRegister64 to ensure the inner GdtRegister64 is correctly
    // aligned as described above.
    struct PACKED AlignedDescPtr64 {
        uint8_t padding[6];
        DescPtr64 reg;

        explicit AlignedDescPtr64(DescPtr64 reg)
            : reg(reg) {
        }
    };
    static_assert(sizeof(AlignedDescPtr64) == 16);

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
        u64 rsp0;
        u64 rsp1;
        u64 rsp2;

        u32 reserved1;
        u32 reserved2;

        // Interrupt stack table pointers.
        //
        // Note that ist[0] corresponds to IST1, i.e. the ist field in an
        // INTERRUPT_GATE descriptor is 1-origin though this array is 0-origin.
        // Descriptors with an ist field of zero do not use the IST at all.
        static constexpr size_t kNumInterruptStackTables = 7;
        u64 ist[kNumInterruptStackTables];

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
        static constexpr u32 kIoMaxBitmapBits = 65536;
        u16 io_port_bitmap_base;
    };
} // namespace arch

#endif // #ifndef ARCH_X86_DESCRIPTOR_HPP
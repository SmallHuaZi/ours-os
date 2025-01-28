#ifndef ARCH_X86_DESCRIPTOR_HPP
#define ARCH_X86_DESCRIPTOR_HPP 1

#include <ours/types.hpp>

namespace arch {
    using ours::u16;
    using ours::u32;
    using ours::u64;
    using ours::usize;

    struct GdteFlags
    {
        // Set by the processor if this segment has been accessed. Only cleared by software.
        // Setting this bit in software prevents GDT writes on first use.
        static constexpr u64 const Accessed = 1ULL << 40;

        // For 32-bit data segments, sets the segment as \c `writable`. For 32-bit code segments,
        // sets the segment as \c `readable`. In 64-bit mode, ignored for all segments.
        static constexpr u64 const Writable = 1ULL << 41;

        // For code segments, sets the segment as “conforming”, influencing the
        // privilege checks that occur on control transfers. For 32-bit data segments,
        // sets the segment as "expand down". In 64-bit mode, ignored for data segments.
        static constexpr u64 const Conforming = 1ULL << 42;

        // This flag must be set for code segments and unset for data segments.
        static constexpr u64 const Executable = 1ULL << 43;

        // This flag must be set for user segments (in contrast to system segments).
        static constexpr u64 const UserSegment = 1ULL << 44;

        // These two bits encode the Entry Privilege Level (DPL) for this descriptor.
        // If both bits are set, the DPL is Ring 3, if both are unset, the DPL is Ring 0.
        static constexpr u64 const DplRing3 = 3ULL << 45;

        // Must be set for any segment, causes a segment not present exception if not set.
        static constexpr u64 const Present = 1ULL << 47;

        // Available for use by the Operating System
        static constexpr u64 const Available = 1ULL << 52;

        // Must be set for 64-bit code segments, unset otherwise.
        static constexpr u64 const LongMode = 1ULL << 53;

        // Use 32-bit (as opposed to 16-bit) operands. If [`LONG_MODE`][Self::LONG_MODE] is set,
        // this must be unset. In 64-bit mode, ignored for data segments.
        static constexpr u64 const DefaultSize = 1ULL << 54;

        // Limit field is scaled by 4096 bytes. In 64-bit mode, ignored for all segments.
        static constexpr u64 const Granularity = 1ULL << 55;

        static constexpr u64 const CommonFlags = UserSegment | Present | Writable | Accessed | Granularity;

        static constexpr u64 const KernelData = CommonFlags | DefaultSize;

        static constexpr u64 const KernelCode32 = CommonFlags | Executable | DefaultSize;

        static constexpr u64 const KernelCode64 = CommonFlags | Executable | LongMode;

        static constexpr u64 const UserData = KernelData | DplRing3;

        static constexpr u64 const UserCode32 = KernelCode32 | DplRing3;

        static constexpr u64 const UserCode64 = KernelCode64 | DplRing3;
    };

    struct Gdte32
    {
        typedef Gdte32     Self;

        constexpr Gdte32(u64 raw = 0)
            : payload(raw)
        {}

        constexpr Gdte32(u32 base, u32 limit, u64 flags)
            : Gdte32()
        {  set(base, limit, flags);  }

        constexpr ~Gdte32() = default;

        constexpr auto set(u32 base, u32 limit, u64 flags) -> void
        {
            set_base(base);
            set_limit(limit);
            set_flags(flags);
        }

        constexpr auto set_base(u32 base) -> void
        {
            payload |= (base & 0x00FFFFFFULL) << 16;
            payload |= (base & 0xFF000000ULL) << 32;
        }

        constexpr auto set_limit(u32 limit) -> void
        {
            payload |= (limit & 0x0000FFFFULL) << 0;
            payload |= (limit & 0x000F0000ULL) << 32;
        }

        constexpr auto set_flags(u64 flags) -> void
        {
            payload |= flags & 0x00F0FF0000000000;
        }

        constexpr auto base() const -> u32
        {
            u32 base = 0;
            base |= (payload >> 16ULL) & 0x00FFFFFF;
            base |= (payload >> 32ULL) & 0xFF000000;
            return base;
        }

        constexpr auto limit() const -> u32
        {
            u32 limit = 0;
            limit |= (payload >> 0ULL) & 0x0000FFFF;
            limit |= (payload >> 32ULL) & 0x000F0000;
            return limit;
        }

        constexpr auto flags() const -> u64
        {
            return payload & 0x00F0FF0000000000;
        }

        constexpr auto value() const -> u64
        {  return payload;  }

        static constexpr auto user_code32() -> Self
        {  return Self(0, 0xfffff, GdteFlags::UserCode32);  }

        static constexpr auto user_data() -> Self
        {  return Self(0, 0xfffff, GdteFlags::UserData);  }

        static constexpr auto kernel_code32() -> Self
        {  return Self(0, 0xfffff, GdteFlags::KernelCode32);  }

        static constexpr auto kernel_data() -> Self
        {  return Self(0, 0xfffff, GdteFlags::KernelData);  }

        u64 payload;
    };

    static_assert(Gdte32::kernel_code32().value() == 0x00cf'9b00'0000'ffff, "");
    static_assert(Gdte32::kernel_data().value() == 0x00cf'9300'0000'ffff, "");
    static_assert(Gdte32::user_code32().value() == 0x00cf'fb00'0000'ffff, "");
    static_assert(Gdte32::user_data().value() == 0x00cf'f300'0000'ffff, "");

    struct Gdte64
        : public Gdte32
    {
        typedef Gdte32    Base;
        typedef Gdte64    Self;

        using Base::Base;

        constexpr Gdte64(u64 base, u64 limit, u64 flags)
            : Self()
        {  set(base, limit, flags);  }

        constexpr ~Gdte64() = default;

        constexpr auto set(u64 base, u64 limit, u64 flags) -> void
        {
            Base::set(base, limit, flags);
            payload64 |= (base >> 32) & 0xFFFFFFFF;
        }

        constexpr auto set_base(u64 base) -> void
        {
            Base::set_base(base);
            payload64 |= (base >> 32) & 0xFFFFFFFF;
        }

        constexpr auto base() const -> u64
        {
            return Base::base() | (payload64 << 32);
        }

        static constexpr auto user_code64() -> Self
        {  return Self(0, 0xfffff, GdteFlags::UserCode64);  }

        static constexpr auto user_code32() -> Self
        {  return Self(0, 0xfffff, GdteFlags::UserCode32);  }

        static constexpr auto user_data() -> Self
        {  return Self(0, 0xfffff, GdteFlags::UserData);  }

        static constexpr auto kernel_code64() -> Self
        {  return Self(0, 0xfffff, GdteFlags::KernelCode64);  }

        static constexpr auto kernel_code32() -> Self
        {  return Self(0, 0xfffff, GdteFlags::KernelCode32);  }

        static constexpr auto kernel_data() -> Self
        {  return Self(0, 0xfffff, GdteFlags::KernelData);  }

    protected:
        u64 payload64;
    };

    static_assert(Gdte64::kernel_code64().value() == 0x00af'9b00'0000'ffff, "");
    static_assert(Gdte64::user_code64().value() == 0x00af'fb00'0000'ffff, "");

    struct IdtEntry32
    {
        typedef IdtEntry32   Base;
        typedef IdtEntry32   Type;

        enum { SelectorShift     = 16ull };
        enum { Offset0To15Shift  = 0ull  };
        enum { Offset16To31Shift = 48ull };

        enum { FlagsMask        = 0x0000FF0000000000ull };
        enum { SelectorMask     = 0x00000000FFFF0000ull };
        enum { Offset0To15Mask  = 0x000000000000FFFFull };
        enum { Offset16To31Mask = 0xFFFF000000000000ull };

        constexpr IdtEntry32()
            : desc0_63(0)
        {}

        constexpr IdtEntry32(u32 selcetor, u32 offset, u32 flags)
            : Base()
        {  set(selcetor, offset, flags);  }

        constexpr auto set(u32 selcetor, u32 offset, u32 flags) -> void
        {
            set_flags(flags);
            set_offset(offset);
            set_selcetor(offset);
        }

        constexpr auto set_flags(u32 flags) -> void
        {
            desc0_63 |= flags & FlagsMask;
        }

        constexpr auto set_offset(u64 offset) -> void
        {
            desc0_63 |= (offset << Offset0To15Shift)  & Offset0To15Mask;
            desc0_63 |= (offset << Offset16To31Shift) & Offset16To31Mask;
        }

        constexpr auto set_selcetor(u32 selcetor) -> void
        {
            desc0_63 |= (selcetor << SelectorShift) & SelectorMask;
        }

        constexpr auto flags() const -> u32
        {
            return desc0_63 & FlagsMask;
        }

        constexpr auto offset() const -> u32
        {
            u32 offset;
            offset = (desc0_63 & Offset0To15Mask) >> Offset0To15Shift;
            offset = (desc0_63 & Offset16To31Mask) >> Offset16To31Shift;
            return offset;
        }

        constexpr auto selector() const -> u32
        {
            return (desc0_63 & SelectorMask) >> SelectorShift;
        }

    protected:
        u64 desc0_63;
    };

    struct IdtEntry64
        : public IdtEntry32
    {
        typedef IdtEntry32   Base;
        typedef IdtEntry64   Self;

        enum { Offset32To63Shift  = 32 };
        enum { Offset32To63Mask   = 0x00000000FFFFFFFF  };

        using Base::Base;

        // BUG
        constexpr IdtEntry64(u32 selcetor, u32 offset, u32 flags)
            : Base()
        {  set(selcetor, offset, flags);  }

        constexpr auto set_offset(u64 offset) -> void
        {
            Base::set_offset(offset);
            desc64_127 |= (offset >> Offset32To63Shift) & Offset32To63Mask;
        }

        constexpr auto offset(u64 offset) const  -> u64
        {
            return Base::offset() | (desc64_127 & Offset32To63Mask) << Offset32To63Shift;
        }

    protected:
        u64 desc64_127;
    };

    template <typename T>
    struct [[gnu::packed]] DescriptorTableBase
    {
        typedef T Entry;
        typedef DescriptorTableBase    Self;

        constexpr DescriptorTableBase() = default;

        constexpr DescriptorTableBase(Entry *entry, u16 n)
            : length(n), table(entry)
        {}

        constexpr auto set_entry(usize idx, Entry entry) -> void
        {  table[idx] = entry;   }

        constexpr auto get_entry(usize idx) const -> Entry
        {  return table[idx];  }

        constexpr auto size() -> usize
        {  return this->length / sizeof(T);  }

    // protected:
        u16    length;
        Entry *table;
    };

    template <typename Entry>
    struct [[gnu::packed]] GdtImpl
        : public DescriptorTableBase<Entry>
    {
        typedef DescriptorTableBase<Entry>   Base;
        typedef GdtImpl       Self;
        using Base::Base;

        auto load() -> void
        {
	        asm volatile("lgdt %0" : : "m" (*this) : "memory");
        }

        static auto current() -> Self
        {
            Self ans;
	        asm volatile("sgdt %0":"=m" (ans));
            return ans;
        }

        static constexpr usize const KERNEL_CODE64 = 1;
        static constexpr usize const KERNEL_CODE32 = 2;
        static constexpr usize const KERNEL_DATA = 3;
        static constexpr usize const USER_CODE64 = 4;
        static constexpr usize const USER_CODE32 = 5;
        static constexpr usize const USER_DATA = 6;
    };

    template <typename T>
    struct [[gnu::packed]] IdtImpl
        : public DescriptorTableBase<T>
    {
        typedef DescriptorTableBase<T>   Base;
        typedef IdtImpl    Self;
        using Base::Base;

        auto load() -> void
        {
	        asm volatile("lidt %0" : : "m" (*this) : "memory");
        }

        static auto current() -> Self
        {
            Self ans;
	        asm volatile("sidt %0" : "=m" (ans));
            return ans;
        }
    };

    typedef IdtImpl<IdtEntry32>     Idt32;
    typedef IdtImpl<IdtEntry64>     Idt64;
    typedef GdtImpl<Gdte32>         Gdt32;
    typedef GdtImpl<Gdte64>         Gdt64;

    // static_assert(sizeof(Gdt32) == 6, "");

    typedef Gdt32   Gdt;
    typedef Gdt32   Idt;

} // namespace arch

#endif // #ifndef ARCH_X86_DESCRIPTOR_HPP
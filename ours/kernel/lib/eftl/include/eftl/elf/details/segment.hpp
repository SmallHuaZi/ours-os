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
#ifndef EFTL_ELF_DETAILS_SEGMENT_HPP
#define EFTL_ELF_DETAILS_SEGMENT_HPP 1

#include <eftl/elf/observable.hpp>
#include <eftl/elf/details/types.hpp>
#include <ustl/util/enum_bits.hpp>

namespace eftl::elf {
    enum class SegPerms {
        None = 0,
        Executable = BIT(0),    // X
        Writable = BIT(1),      // W
        Readable = BIT(2),      // R
    };
    USTL_ENABLE_ENUM_BITMASK(SegPerms);

    enum class SegType : u64 {
        Unknown = u64(-1),
        Null = 0,     //< Unused segment. 
        Load = 1,     //< Loadable segment.
        Dynamic = 2,  //< Dynamic linking information. 
        Interp = 3,   //< Interpreter pathname.
        Note = 4,     //< Auxiliary information.
        Shlib = 5,    //< Reserved.
        Phdr = 6,     //< The program header table itself.
        Tls = 7,      //< The thread-local storage template.

        GnuEhFrame = 0x6474e550,
        GnuStack = 0x6474e551,    //< Indicates stack executability.
        GnuProperty = 0x6474e553, //< GNU property. 
        GnuRelro = 0x6474e552,    //< Read-only after relocation.
        PaxFlags = 0x65041580,
    };

    /// CRTP class help us define a group of consistent interface.
    template <typename ElfTraits, typename Derived>
    struct SegmentBase: public Observable <ElfTraits, Derived> {
        FORCE_INLINE CXX11_CONSTEXPR
        auto va_begin() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_vaddr;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto va_end() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_vaddr +
                   reinterpret_cast<Derived const *>(this)->p_memsz;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto va_size() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_memsz;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto file_offset() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_offset;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto file_size() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_filesz;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto alignment() const -> usize {
            return reinterpret_cast<Derived const *>(this)->p_align;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto permissions() const -> SegPerms {
            return static_cast<SegPerms>(reinterpret_cast<Derived const *>(this)->p_flags);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto type() const -> SegType {
            return static_cast<SegType>(reinterpret_cast<Derived const *>(this)->p_type);
        }
    };

    /** Program header for ELF32. */
    struct Elf32Phdr: public SegmentBase<ElfTraits32, Elf32Phdr> {
        Elf32Word p_type;   /**< Type of segment */
        Elf32Off p_offset;  /**< File offset where segment is located, in bytes */
        Elf32Addr p_vaddr;  /**< Virtual address of beginning of segment */
        Elf32Addr p_paddr;  /**< Physical address of beginning of segment (OS-specific) */
        Elf32Word p_filesz; /**< Num. of bytes in file image of segment (may be zero) */
        Elf32Word p_memsz;  /**< Num. of bytes in mem image of segment (may be zero) */
        Elf32Word p_flags;  /**< Segment flags */
        Elf32Word p_align;  /**< Segment alignment constraint */
    };

    /** Program header for ELF64. */
    struct Elf64Phdr: public SegmentBase<ElfTraits64, Elf64Phdr> {
        Elf64Word p_type;    /**< Type of segment */
        Elf64Word p_flags;   /**< Segment flags */
        Elf64Off p_offset;   /**< File offset where segment is located, in bytes */
        Elf64Addr p_vaddr;   /**< Virtual address of beginning of segment */
        Elf64Addr p_paddr;   /**< Physical addr of beginning of segment (OS-specific) */
        Elf64Xword p_filesz; /**< Num. of bytes in file image of segment (may be zero) */
        Elf64Xword p_memsz;  /**< Num. of bytes in mem image of segment (may be zero) */
        Elf64Xword p_align;  /**< Segment alignment constraint */
    };

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_DETAILS_SEGMENT_HPP
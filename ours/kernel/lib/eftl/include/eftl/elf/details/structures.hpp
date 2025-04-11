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
#ifndef EFTL_ELF_DETAILS_STRUCTURES_HPP
#define EFTL_ELF_DETAILS_STRUCTURES_HPP 1

#include <ours/types.hpp>
#include <ours/config.hpp>

namespace eftl::elf {
    typedef u32 Elf32Addr;
    typedef u32 Elf32Off;
    typedef u16 Elf32Half;
    typedef u32 Elf32Word;
    typedef i32 Elf32Sword;

    typedef u64 Elf64Addr;
    typedef u64 Elf64Off;
    typedef u16 Elf64Half;
    typedef u32 Elf64Word;
    typedef i32 Elf64Sword;

    typedef u64 Elf64Xword;
    typedef i64 Elf64Sxword;

    /** Object file magic string. */
    static const char kElfMagic[] = {'\x7f', 'E', 'L', 'F'};

    /** 32-bits ELF header */
    struct Elf32Ehdr {
        unsigned char e_ident[16]; /**< ELF Identification bytes */
        Elf32Half e_type;          /**< Type of file (see ET_* below) */
        Elf32Half e_machine;       /**< Required architecture for this file (see EM_*) */
        Elf32Word e_version;       /**< Must be equal to 1 */
        Elf32Addr e_entry;         /**< Address to jump to in order to start program */
        Elf32Off e_phoff;          /**< Program header table's file offset, in bytes */
        Elf32Off e_shoff;          /**< Section header table's file offset, in bytes */
        Elf32Word e_flags;         /**< Processor-specific flags */
        Elf32Half e_ehsize;        /**< Size of ELF header, in bytes */
        Elf32Half e_phentsize;     /**< Size of an entry in the program header table */
        Elf32Half e_phnum;         /**< Number of entries in the program header table */
        Elf32Half e_shentsize;     /**< Size of an entry in the section header table */
        Elf32Half e_shnum;         /**< Number of entries in the section header table */
        Elf32Half e_shstrndx;      /**< Sect hdr table index of sect name string table */
    };

    /**
     * @brief 64-bit ELF header. Fields are the same as for Elf32Ehdr, but with different
     * types
     * @see Elf32Ehdr
     */
    struct Elf64Ehdr {
        unsigned char e_ident[16];
        Elf64Half e_type;
        Elf64Half e_machine;
        Elf64Word e_version;
        Elf64Addr e_entry;
        Elf64Off e_phoff;
        Elf64Off e_shoff;
        Elf64Word e_flags;
        Elf64Half e_ehsize;
        Elf64Half e_phentsize;
        Elf64Half e_phnum;
        Elf64Half e_shentsize;
        Elf64Half e_shnum;
        Elf64Half e_shstrndx;
    };

    /** 32-bits Section header. */
    struct Elf32Shdr {
        Elf32Word sh_name;      /**< Section name (index into string table) */
        Elf32Word sh_type;      /**< Section type (SHT_*) */
        Elf32Word sh_flags;     /**< Section flags (SHF_*) */
        Elf32Addr sh_addr;      /**< Address where section is to be loaded */
        Elf32Off sh_offset;     /**< File offset of section data, in bytes */
        Elf32Word sh_size;      /**< Size of section, in bytes */
        Elf32Word sh_link;      /**< Section type-specific header table index link */
        Elf32Word sh_info;      /**< Section type-specific extra information */
        Elf32Word sh_addralign; /**< Section address alignment */
        Elf32Word sh_entsize;   /**< Size of records contained within the section */
    };

    /**
     * 64-bits Section header Section header for ELF64 - same fields as Elf32Shdr, different types.
     * @see Elf32Shdr
     */
    struct Elf64Shdr {
        Elf64Word sh_name;
        Elf64Word sh_type;
        Elf64Xword sh_flags;
        Elf64Addr sh_addr;
        Elf64Off sh_offset;
        Elf64Xword sh_size;
        Elf64Word sh_link;
        Elf64Word sh_info;
        Elf64Xword sh_addralign;
        Elf64Xword sh_entsize;
    };

    /** Symbol table entries for ELF32. */
    struct Elf32Sym {
        Elf32Word st_name;      /**< Symbol name (index into string table) */
        Elf32Addr st_value;     /**< Value or address associated with the symbol */
        Elf32Word st_size;      /**< Size of the symbol */
        unsigned char st_info;  /**< Symbol's type and binding attributes */
        unsigned char st_other; /**< Symbol visibility */
        Elf32Half st_shndx;     /**< Which section (header table index) it's defined in */
    };

    /** Symbol table entries for ELF64. */
    struct Elf64Sym {
        Elf64Word st_name;      /**< Symbol name (index into string table) */
        unsigned char st_info;  /**< Symbol's type and binding attributes */
        unsigned char st_other; /**< Symbol visibility */
        Elf64Half st_shndx;     /**< Which section (header tbl index) it's defined in */
        Elf64Addr st_value;     /**< Value or address associated with the symbol */
        Elf64Xword st_size;     /**< Size of the symbol */
    };

    /** @brief Relocation entry, without explicit addend. */
    struct Elf32Rel {
        Elf32Addr r_offset; /**< Location (file byte offset, or program virtual addr) */
        Elf32Word r_info;   /**< Symbol table index and type of relocation to apply */
    };

    /** @brief Relocation entry with explicit addend. */
    struct Elf32Rela {
        Elf32Addr r_offset;  /**< Location (file byte offset, or program virtual addr) */
        Elf32Word r_info;    /**< Symbol table index and type of relocation to apply */
        Elf32Sword r_addend; /**< Compute value for relocatable field by adding this */
    };

    /** @brief Relocation entry, without explicit addend. */
    struct Elf64Rel {
        Elf64Addr r_offset; /**< Location (file byte offset, or program virtual addr). */
        Elf64Xword r_info;  /**< Symbol table index and type of relocation to apply. */
    };

    /** @brief Relocation entry with explicit addend. */
    struct Elf64Rela {
        Elf64Addr r_offset;   /**< Location (file byte offset, or program virtual addr). */
        Elf64Xword r_info;    /**< Symbol table index and type of relocation to apply. */
        Elf64Sxword r_addend; /**< Compute value for relocatable field by adding this. */
    };

    struct Elf32Verneed {
        Elf32Half vn_version; // Version of structure */
        Elf32Half vn_cnt;     // Number of associated aux entry */
        Elf32Word vn_file;    // Offset of filename for this dependency */
        Elf32Word vn_aux;     // Offset in bytes to vernaux array */
        Elf32Word vn_next;    // Offset in bytes to next verneed */
    };

    struct Elf64Verneed {
        Elf64Half vn_version; // Version of structure */
        Elf64Half vn_cnt;     // Number of associated aux entry */
        Elf64Word vn_file;    // Offset of filename for this dependency */
        Elf64Word vn_aux;     // Offset in bytes to vernaux array */
        Elf64Word vn_next;    // Offset in bytes to next verneed */
    };

    struct Elf64Vernaux {
        Elf64Word vna_hash;
        Elf64Half vna_flags;
        Elf64Half vna_other;
        Elf64Word vna_name;
        Elf64Word vna_next;
    };

    struct Elf32Vernaux {
        Elf32Word vna_hash;
        Elf32Half vna_flags;
        Elf32Half vna_other;
        Elf32Word vna_name;
        Elf32Word vna_next;
    };

    struct Elf32Auxv {
        u32 a_type; /**< Entry type */
        union {
            u32 a_val; /**< Integer value */
        } a_un;
    };

    struct Elf64Auxv {
        u64 a_type; /**< Entry type */
        union {
            u64 a_val; /**< Integer value */
        } a_un;
    };

    /** Structure for .gnu.version_d (64 bits) */
    struct Elf64Verdef {
        Elf64Half vd_version; /**< Version revision */
        Elf64Half vd_flags;   /**< Version information */
        Elf64Half vd_ndx;     /**< Version Index */
        Elf64Half vd_cnt;     /**< Number of associated aux entries */
        Elf64Word vd_hash;    /**< Version name hash value */
        Elf64Word vd_aux;     /**< Offset in bytes to verdaux array */
        Elf64Word vd_next;    /**< Offset in bytes to next verdef entry */
    };

    /** Structure for .gnu.version_d (32 bits) */
    struct Elf32Verdef {
        Elf32Half vd_version; /**< Version revision */
        Elf32Half vd_flags;   /**< Version information */
        Elf32Half vd_ndx;     /**< Version Index */
        Elf32Half vd_cnt;     /**< Number of associated aux entries */
        Elf32Word vd_hash;    /**< Version name hash value */
        Elf32Word vd_aux;     /**< Offset in bytes to verdaux array */
        Elf32Word vd_next;    /**< Offset in bytes to next verdef entry */
    };

    struct Elf64Verdaux {
        Elf64Word vda_name; /**< Version or dependency names */
        Elf64Word vda_next; /**< Offset in bytes to next verdaux entry */
    };

    struct Elf32Verdaux {
        Elf32Word vda_name; /**< Version or dependency names */
        Elf32Word vda_next; /**< Offset in bytes to next verdaux entry */
    };

    struct ElfSiginfo {
        i32 si_signo;
        i32 si_code;
        i32 si_errno;
    };

    struct Elf32Timeval {
        u32 tv_sec;
        u32 tv_usec;
    };

    struct Elf64Timeval {
        u64 tv_sec;
        u64 tv_usec;
    };

    /** Structure for note type NT_PRSTATUS */
    struct Elf32Prstatus {
        ElfSiginfo pr_info;
        u16 pr_cursig;
        u16 reserved;

        u32 pr_sigpend;
        u32 pr_sighold;

        i32 pr_pid;
        i32 pr_ppid;
        i32 pr_pgrp;
        i32 pr_sid;

        Elf32Timeval pr_utime;
        Elf32Timeval pr_stime;
        Elf32Timeval pr_cutime;
        Elf32Timeval pr_cstime;
    };

    struct Elf64Prstatus {
        ElfSiginfo pr_info;
        u16 pr_cursig;
        u16 reserved;

        u64 pr_sigpend;
        u64 pr_sighold;

        i32 pr_pid;
        i32 pr_ppid;
        i32 pr_pgrp;
        i32 pr_sid;

        Elf64Timeval pr_utime;
        Elf64Timeval pr_stime;
        Elf64Timeval pr_cutime;
        Elf64Timeval pr_cstime;
    };

    /** Structure for note type NT_FILE */
    struct Elf64FileEntry {
        u64 start;
        u64 end;
        u64 file_ofs;
    };

    struct Elf32FileEntry {
        u32 start;
        u32 end;
        u32 file_ofs;
    };

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_DETAILS_STRUCTURES_HPP
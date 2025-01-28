#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <ours/type.hpp>

namespace ours {
    // The magic number for the Multiboot header 
    constexpr static u32 const MBH_MAGIC = 0x1BADB002;

    //  The magic number passed by a Multiboot-compliant boot loader
    //  Must be passed in register %eax when jumping to the Multiboot OS
    //  image.
    constexpr static u32 const MB_BOOTLOADER_MAGIC = 0x2BADB002;

    enum MbhFlags: u32 {
        // Boot modules must be page aligned 
        PageAlign = 0x00000001,

        // Must pass memory information to OS
        MemoryInfo = 0x00000002,

        // Must pass video information to OS
        VideoMode = 0x00000004,

        // Image is a raw Multiboot image (not ELF) 
        RawImage = 0x00010000,

        Acpi = 0x00
    };

    // If bit 0 in the `flags' word is set, then the `mem_*' fields are
    // valid. `mem_lower' and `mem_upper' indicate the amount of lower and
    // upper memory, respectively, in kilobytes. Lower memory starts at
    // address 0, and upper memory starts at address 1 megabyte. The maximum
    // possible value for lower memory is 640 kilobytes. The value returned for
    // upper memory is maximally the address of the first upper memory hole
    // minus 1 megabyte. It is not guaranteed to be this value.
    enum MbiFlags: u32 {
        // Multiboot information structure mem_* fields are valid 
        Mem = 0x00000001,

        // Multiboot information structure boot_device field is valid 
        Bootdev = 0x00000002,

        // Multiboot information structure cmdline field is valid 
        Cmdline = 0x00000004,

        // Multiboot information structure module fields are valid 
        Mods = 0x00000008,

        // Multiboot information structure a.out symbol table is valid 
        Aout = 0x00000010,

        // Multiboot information struture ELF section header table is valid 
        Elf = 0x00000020,

        // Multiboot information structure memory map is valid 
        Mmap = 0x00000040,

        // Multiboot information structure drive list is valid 
        Drives = 0x00000080,

        // Multiboot information structure ROM configuration field is valid 
        Cfgtbl = 0x00000100,

        // Multiboot information structure boot loader name field is valid 
        Loader = 0x00000200,

        // Multiboot information structure APM table is valid 
        Apm = 0x00000400,

        // Multiboot information structure video information is valid 
        Vbe = 0x00000800,
    };

    // A Multiboot header 
    struct MultibootHeader {
        u32 magic;
        u32 flags;
        u32 checksum;
        u32 header_addr;
        u32 load_addr;
        u32 load_end_addr;
        u32 bss_end_addr;
        u32 entry_addr;
    } __attribute__ (( packed, may_alias ));

    // A Multiboot a.out symbol table 
    struct MultibootAoutSymbolTable {
        u32 tabsize;
        u32 strsize;
        u32 addr;
        u32 reserved;
    } __attribute__ (( packed, may_alias ));

    // A Multiboot ELF section header table 
    struct MultibootElfSectionHeaderTable {
        u32 num;
        u32 size;
        u32 addr;
        u32 shndx;
    } __attribute__ (( packed, may_alias ));

    // A Multiboot information structure 
    struct MultibootInfo {
        u32 flags;
        u32 mem_lower;
        u32 mem_upper;
        u32 boot_device;
        u32 cmdline;
        u32 mods_count;
        u32 mods_addr;
        union {
            struct MultibootAoutSymbolTable  aout_syms;
            struct MultibootElfSectionHeaderTable  elf_sections;
        } syms;
        u32 mmap_length;
        u32 mmap_addr;
        u32 drives_length;
        u32 drives_addr;
        u32 config_table;
        u32 boot_loader_name;
        u32 apm_table;
        u32 vbe_control_info;
        u32 vbe_mode_info;
        u16 vbe_mode;
        u16 vbe_interface_seg;
        u16 vbe_interface_off;
        u16 vbe_interface_len;
    } __attribute__ (( packed, may_alias ));

    // A Multiboot module structure 
    struct MultibootModule {
        u32 mod_start;
        u32 mod_end;
        u32 string;
        u32 reserved;
    } __attribute__ (( packed, may_alias ));

    enum MbMmapFlags: u32 {
        Available = 1,
        Reserved  = 2,
        AcpiReclaimable = 3,
        Nvs = 4,
        Badram = 5,
    };

    // A Multiboot memory map entry 
    struct MultibootMemoryMap {
        u32 size;
        u64 addr;
        u64 len;
        u32 type;
    } __attribute__ (( packed, may_alias ));

}

#endif // MULTIBOOT_H 
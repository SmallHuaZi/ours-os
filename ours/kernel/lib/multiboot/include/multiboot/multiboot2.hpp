/*   Multiboot2.h - Multiboot 2 header file. */
/*   Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OURS_MULTIBOOT2
#define OURS_MULTIBOOT2 1

/*  How many bytes from the start of the file we search for the header. */
#define MULTIBOOT_SEARCH                        32768
#define MULTIBOOTHEADER_ALIGN                  8

/*  The magic field should contain this. */
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6

/*  This should be in %eax. */
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

/*  Alignment of Multiboot modules. */
#define MULTIBOOT_MOD_ALIGN                     0x00001000

/*  Alignment of the Multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN                    0x00000008

/*  Flags set in the ’flags’ member of the Multiboot header. */

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT_TAG_TYPE_EFI_BS            18
#define MULTIBOOT_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    21

#define MULTIBOOT_HEADER_TAG_END  0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT_HEADER_TAG_ADDRESS  2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN  6
#define MULTIBOOT_HEADER_TAG_EFI_BS        7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE  10

#define MULTIBOOT_ARCHITECTURE_I386  0
#define MULTIBOOT_ARCHITECTURE_MIPS32  4
#define MULTIBOOT_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT_LOAD_PREFERENCE_None 0
#define MULTIBOOT_LOAD_PREFERENCE_LOW 1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH 2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#ifndef ASM

#include <ours/types.hpp>

namespace multiboot2 {
    using ours::u8;
    using ours::u16;
    using ours::u32;
    using ours::u64;

    struct MultibootHeader {
        //  Must be MULTIBOOT_MAGIC - see above.
        u32 magic;

        //  ISA
        u32 architecture;

        //  Total header length.
        u32 header_length;

        //  The above fields plus this one must equal 0 mod 2^32.
        u32 checksum;
    };

    struct MultibootHeaderTag {
        u16 type;
        u16 flags;
        u32 size;
    };

    struct MultibootHeaderTagInformationRequest {
        u16 type;
        u16 flags;
        u32 size;
        u32 requests[0];
    };

    struct MultibootHeaderTagAddress {
        u16 type;
        u16 flags;
        u32 size;
        u32 header_addr;
        u32 load_addr;
        u32 load_end_addr;
        u32 bss_end_addr;
    };

    struct MultibootHeaderTagEntryAddress {
        u16 type;
        u16 flags;
        u32 size;
        u32 entry_addr;
    };

    struct MultibootHeaderTagConsoleFlags {
        u16 type;
        u16 flags;
        u32 size;
        u32 console_flags;
    };

    struct MultibootHeaderTagFramebuffer
    {
        u16 type;
        u16 flags;
        u32 size;
        u32 width;
        u32 height;
        u32 depth;
    };

    struct MultibootHeaderTagModuleAlign
    {
        u16 type;
        u16 flags;
        u32 size;
    };

    struct MultibootHeaderTagRelocatable
    {
        u16 type;
        u16 flags;
        u32 size;
        u32 min_addr;
        u32 max_addr;
        u32 align;
        u32 preference;
    };

    struct MultibootColor
    {
        u8 red;
        u8 green;
        u8 blue;
    };

    struct MultibootMmapEntry
    {
        enum {  
            Available = 1,
            Reserved = 2,
            AcpiReclaimable = 3,
            Nvs = 4,
            BadRam = 5,
        };

        u64 addr;
        u64 len;
        u32 type;
        u32 zero;
    };
    typedef struct MultibootMmapEntry MultibootMemoryMap;

    struct MultibootTag
    {
        u32 type;
        u32 size;
    };

    struct MultibootTagString
    {
        u32 type;
        u32 size;
        char string[0];
    };

    struct MultibootTagModule
    {
        u32 type;
        u32 size;
        u32 mod_start;
        u32 mod_end;
        char cmdline[0];
    };

    struct MultibootTagBasicMeminfo
    {
        u32 type;
        u32 size;
        u32 mem_lower;
        u32 mem_upper;
    };

    struct MultibootTagBootdev
    {
        u32 type;
        u32 size;
        u32 biosdev;
        u32 slice;
        u32 part;
    };

    struct MultibootTagMmap
    {
        u32 type;
        u32 size;
        u32 entry_size;
        u32 entry_version;
        struct MultibootMmapEntry entries[0];  
    };

    struct MultibootVbeInfoBlock
    {
        u8 external_specification[512];
    };

    struct MultibootVbeModeInfoBlock
    {
        u8 external_specification[256];
    };

    struct MultibootTagVbe
    {
        u32 type;
        u32 size;

        u16 vbe_mode;
        u16 vbe_interface_seg;
        u16 vbe_interface_off;
        u16 vbe_interface_len;

        struct MultibootVbeInfoBlock vbe_control_info;
        struct MultibootVbeModeInfoBlock vbe_mode_info;
    };

    struct MultibootTagFramebufferCommon
    {
        u32 type;
        u32 size;

        u64 framebuffer_addr;
        u32 framebuffer_pitch;
        u32 framebuffer_width;
        u32 framebuffer_height;
        u8 framebuffer_bpp;

        enum {
            Indexed,
            Rgb,
            EgaText,
        };
        u8 framebuffer_type;
        u16 reserved;
    };

    struct MultibootTagFramebuffer
    {
        struct MultibootTagFramebufferCommon common;

        union {
            struct {
                u16 framebuffer_palette_num_colors;
                struct MultibootColor framebuffer_palette[0];
            };
            struct {
                u8 framebuffer_red_field_position;
                u8 framebuffer_red_mask_size;
                u8 framebuffer_green_field_position;
                u8 framebuffer_green_mask_size;
                u8 framebuffer_blue_field_position;
                u8 framebuffer_blue_mask_size;
            };
      };
    };

    struct MultibootTagElfSections
    {
        u32 type;
        u32 size;
        u32 num;
        u32 entsize;
        u32 shndx;
        char sections[0];
    };

    struct MultibootTagApm
    {
        u32 type;
        u32 size;
        u16 version;
        u16 cseg;
        u32 offset;
        u16 cseg_16;
        u16 dseg;
        u16 flags;
        u16 cseg_len;
        u16 cseg16_len;
        u16 dseg_len;
    };

    struct MultibootTagEfi32
    {
        u32 type;
        u32 size;
        u32 pointer;
    };

    struct MultibootTagEfi64
    {
        u32 type;
        u32 size;
        u64 pointer;
    };

    struct MultibootTagSmbios
    {
        u32 type;
        u32 size;
        u8 major;
        u8 minor;
        u8 reserved[6];
        u8 tables[0];
    };

    struct MultibootTagOldAcpi
    {
        u32 type;
        u32 size;
        u8 rsdp[0];
    };

    struct MultibootTagNewAcpi
    {
        u32 type;
        u32 size;
        u8 rsdp[0];
    };

    struct MultibootTagNetwork
    {
        u32 type;
        u32 size;
        u8 dhcpack[0];
    };

    struct MultibootTagEfiMmap
    {
        u32 type;
        u32 size;
        u32 descr_size;
        u32 descr_vers;
        u8 efi_mmap[0];
    }; 

    struct MultibootTagEfi32Ih
    {
        u32 type;
        u32 size;
        u32 pointer;
    };

    struct MultibootTagEfi64Ih
    {
        u32 type;
        u32 size;
        u64 pointer;
    };

    struct MultibootTagLoadBaseAddr
    {
        u32 type;
        u32 size;
        u32 load_base_addr;
    };
}

#endif // ASM
#endif // OURS_MULTIBOOT2

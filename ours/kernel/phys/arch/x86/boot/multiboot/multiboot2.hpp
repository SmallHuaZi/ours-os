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

#include <ours/types.hpp>

#include "defines.hpp"

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
        MaxNumType,
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

#endif // OURS_MULTIBOOT2
